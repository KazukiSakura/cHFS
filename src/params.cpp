#include "params.hpp"
#include <climits>
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include "actogram.hpp"
#include "container.hpp"
#include "hfs.hpp"

#define DEFAULT_CYCLE 24.0
#define DEFAULT_RESOLUTION 0.1
#define DEFAULT_WEIGHT 1.0

namespace chfs {
namespace {
auto split_line(std::string_view line, std::string_view delim) {
  Vector<std::string_view> result;
  while (!line.empty()) {
    auto pos = line.find_first_of(delim);
    result.push_back(line.substr(0, pos));
    line.remove_prefix(pos < line.size() ? pos + 1 : line.size());
  }
  return result;
}
std::optional<int> to_int(std::string_view view) {
  if (!view.empty()) {
    std::string str{view};
    char *pos = nullptr;
    auto val = std::strtol(str.c_str(), &pos, 10);
    if (pos && !*pos && LONG_MIN < val && val < LONG_MAX) {
      return val;
    }
  }
  return std::nullopt;
}
std::optional<double> to_double(std::string_view view) {
  if (!view.empty()) {
    std::string str{view};
    char *pos = nullptr;
    auto val = std::strtod(str.c_str(), &pos);
    if (pos && !*pos && std::isfinite(val)) {
      return val;
    }
  }
  return std::nullopt;
}
Result<Vector<int>> get_csv(std::istream &is) {
  Vector<int> csv;
  std::string line;
  for (int row = 1; is.good(); ++row) {
    std::getline(is, line);
    for (const auto &e : split_line(line, ",")) {
      if (auto v = to_int(e)) {
        csv.push_back(*v);
      } else {
        return emit_error<error::InvalidInput>(row);
      }
    }
  }
  return ok(csv);
}

std::optional<Error> set_double(double &dst, std::string_view v) {
  if (auto d = to_double(v)) {
    dst = *d;
    return std::nullopt;
  }
  return error::InvalidOption{std::string{v}};
}
template <typename T>
std::optional<Error> set_file(std::optional<T> &fs, std::string_view v) {
  if (v != "-") {
    if (T file{std::string{v}}) {
      fs = std::move(file);
    } else {
      return error::OpenFailure{std::string{v}};
    }
  }
  return std::nullopt;
}
}  // namespace

Parameters::Parameters()
    : cycle{DEFAULT_CYCLE},
      reso{DEFAULT_RESOLUTION},
      weight{DEFAULT_WEIGHT},
      verbose{0},
      method{do_chfs} {}
std::optional<Error> Parameters::set_cycle(std::string_view v) {
  return set_double(cycle, v);
}
std::optional<Error> Parameters::set_resolution(std::string_view v) {
  return set_double(reso, v);
}
std::optional<Error> Parameters::set_weight(std::string_view v) {
  return set_double(weight, v);
}
std::optional<Error> Parameters::set_verbose(std::string_view v) {
  if (auto i = to_int(v); i && 1 <= *i && *i <= 5) {
    verbose = *i;
    return std::nullopt;
  }
  return error::InvalidOption{std::string{v}};
}
std::optional<Error> Parameters::set_input(std::string_view v) {
  return set_file(ifs, v);
}
std::optional<Error> Parameters::set_output(std::string_view v) {
  return set_file(ofs, v);
}
void Parameters::set_verbose() { verbose = true; }
void Parameters::set_method() { method = do_hfs; }
Result<int> Parameters::run() {
  if (auto csv = get_csv(ifs ? *ifs : std::cin)) {
    auto &os = ofs ? *ofs : std::cout;
    auto result = method(Actogram{*csv, cycle, reso, weight}, verbose);
    if (result.is<picojson::null>()) {
      return emit_error<error::ProcessFailure>();
    } else if (0 == verbose) {
      auto data = result.serialize();
      os << data.substr(1, data.size() - 2) << std::endl;
    } else {
      picojson::object json;
      json["cycle"] = to_json(cycle, verbose);
      json["resolution"] = to_json(reso, verbose);
      json["weight"] = to_json(weight, verbose);
      json_merge(json, result);
      os << Json{json}.serialize(true);
    }
    return ok(0);
  } else {
    return err(csv);
  }
}

namespace {
class Options {
  std::unordered_set<char> flags;
  std::unordered_map<char, std::string_view> params;

 public:
  void add_flag(char key) { flags.insert(key); }
  void add_param(char key, std::string_view param) {
    params.try_emplace(key, param);
  }
  bool get_flag(char key) const { return 0 < flags.count(key); }
  std::optional<std::string_view> get_param(char key) const {
    if (auto it = params.find(key); it != params.end()) {
      return it->second;
    }
    return std::nullopt;
  }
};

class OptionParser {
  std::unordered_map<char, bool> spec;

 public:
  OptionParser(std::string_view flags, std::string_view params) {
    for (const auto &k : flags) {
      spec.try_emplace(k, true);
    }
    for (const auto &k : params) {
      spec.try_emplace(k, false);
    }
  }
  Result<Options> parse(int argc, char *const *argv) {
    Options options;
    for (int i = 1; i < argc; ++i) {
      if (auto opt = small_option(argv[i])) {
        auto k = *opt;
        if (spec[k]) {
          options.add_flag(k);
        } else if (i + 1 < argc) {
          options.add_param(k, argv[++i]);
        } else {
          return emit_error<error::MissingParam>(argv[i]);
        }
      } else {
        return err(opt);
      }
    }
    return ok(options);
  }
  Result<char> small_option(std::string_view arg) const {
    if (arg.empty() || arg[0] != '-') {
      return emit_error<error::MissingOption>(std::string{arg});
    } else if (arg.size() != 2 || 0 == spec.count(arg[1])) {
      return emit_error<error::UnknownOption>(std::string{arg});
    } else {
      return ok(arg[1]);
    }
  }
};
template <typename T>
using SetterList = std::initializer_list<std::tuple<char, T>>;
}  // namespace

Result<Parameters> parse_args(int argc, char *const *argv) {
  if (argc == 1) {
    return emit_error<error::Help>();
  }
  OptionParser parser{"ah", "focrvw"};
  if (auto opts = parser.parse(argc, argv)) {
    Parameters params;
    if (opts->get_flag('h')) {
      return emit_error<error::Help>();
    }
    using FlagSetter = void (Parameters::*)();
    static const SetterList<FlagSetter> set_flags = {
        {'a', &Parameters::set_method},
    };
    for (const auto &[k, f] : set_flags) {
      if (opts->get_flag(k)) {
        (params.*f)();
      }
    }
    using ParamSetter = std::optional<Error> (Parameters::*)(std::string_view);
    static const SetterList<ParamSetter> set_params = {
        {'f', &Parameters::set_input},  {'o', &Parameters::set_output},
        {'c', &Parameters::set_cycle},  {'r', &Parameters::set_resolution},
        {'w', &Parameters::set_weight}, {'v', &Parameters::set_verbose},
    };
    for (const auto &[k, f] : set_params) {
      if (auto v = opts->get_param(k)) {
        if (auto e = (params.*f)(*v)) {
          return err(*e);
        }
      }
    }
    if (!opts->get_param('f')) {
      return emit_error<error::NoInput>();
    }
    return ok(std::move(params));
  } else {
    return err(opts);
  }
}
#define TO_STR(x) TO_STR_AUX(x)
#define TO_STR_AUX(x) #x
void usage(std::ostream &os, std::string_view cmd) {
  static const auto msg = {" [OPTION]...",
                           "OPTION:",
                           "  -f FILE   input csv file (mandatory)",
                           "  -o FILE   output file (default: standard output)",
                           "  -c VALUE  expected cycle in hours "
                           "(default: " TO_STR(DEFAULT_CYCLE) ")",
                           "  -r VALUE  input resolution in hours "
                           "(default: " TO_STR(DEFAULT_RESOLUTION) ")",
                           "  -w VALUE  penalty weight "
                           "(default: " TO_STR(DEFAULT_WEIGHT) ")",
                           "  -v VALUE  verbose level (1-5)",
                           "  -a        apply HFS method to whole actogram",
                           "  -h        print this help"};
  os << "USAGE: " << cmd;
  for (const auto &line : msg) {
    os << line << std::endl;
  }
}

}  // namespace chfs
