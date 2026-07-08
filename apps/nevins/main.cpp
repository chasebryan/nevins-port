#include "nevins/capture.h"
#include "nevins/core_api.h"
#include "nevins/device.h"
#include "nevins/signal_card.h"
#include "nevins/version.h"

#include <cstdint>
#include <exception>
#include <filesystem>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

namespace {

void print_usage() {
  std::cout
      << "Nevins Port - " << nevins::kTagline << "\n\n"
      << "Usage:\n"
      << "  nevins --version\n"
      << "  nevins probe --mock\n"
      << "  nevins survey --mock --duration 5 --out build/demo.nvcap\n"
      << "  nevins identify --mock --freq 433920000\n"
      << "  nevins explain build/demo.nvcap\n"
      << "  nevins replay build/demo.nvcap\n";
}

bool has_arg(const std::vector<std::string>& args, const std::string& value) {
  for (const auto& arg : args) {
    if (arg == value) {
      return true;
    }
  }
  return false;
}

std::optional<std::string> value_after(const std::vector<std::string>& args, const std::string& key) {
  for (std::size_t index = 0; index + 1U < args.size(); ++index) {
    if (args[index] == key) {
      return args[index + 1U];
    }
  }
  return std::nullopt;
}

std::uint32_t parse_u32_arg(const std::string& value, const std::string& name) {
  std::size_t consumed = 0;
  const auto parsed = std::stoul(value, &consumed, 10);
  if (consumed != value.size() || parsed > UINT32_MAX) {
    throw std::runtime_error("invalid " + name + " value");
  }
  return static_cast<std::uint32_t>(parsed);
}

std::uint64_t parse_u64_arg(const std::string& value, const std::string& name) {
  std::size_t consumed = 0;
  const auto parsed = std::stoull(value, &consumed, 10);
  if (consumed != value.size()) {
    throw std::runtime_error("invalid " + name + " value");
  }
  return parsed;
}

int probe_mock() {
  const nevins::MockSdrSource source;
  const auto status = source.status();
  std::cout << "DEVICE PROBE\n\n";
  std::cout << "Mode: " << status.mode << '\n';
  std::cout << "Receiver: " << status.name << '\n';
  std::cout << "Hardware required: no\n";
  std::cout << "Receive-only: yes\n";
  std::cout << "Frequency limits: " << nevins::format_frequency_mhz(status.min_frequency_hz) << " to "
            << nevins::format_frequency_mhz(status.max_frequency_hz) << '\n';
  std::cout << "Sample-rate limits: " << status.min_sample_rate_hz << " to " << status.max_sample_rate_hz << " Hz\n";
  std::cout << "Policy: no transmit, no spoofing, no jamming, no protected-content decoding\n";
  return 0;
}

int survey_mock(const std::vector<std::string>& args) {
  const auto duration_arg = value_after(args, "--duration").value_or("5");
  const auto output_arg = value_after(args, "--out").value_or("build/demo.nvcap");
  const auto duration_seconds = parse_u32_arg(duration_arg, "duration");
  const auto pack = nevins::create_mock_capture(output_arg, duration_seconds);

  std::cout << "SURVEY COMPLETE\n\n";
  std::cout << "Mode: mock\n";
  std::cout << "Capture pack: " << std::filesystem::absolute(pack.root).string() << '\n';
  std::cout << "IQ bytes: " << pack.iq.size() << '\n';
  std::cout << "Signal card: " << (pack.root / pack.manifest.signal_card_file).string() << '\n';
  std::cout << "Category: " << nv_signal_category_name(pack.card.category) << '\n';
  std::cout << "Policy: receive-only analysis; content decoding not attempted\n";
  return 0;
}

int identify_mock(const std::vector<std::string>& args) {
  const auto freq_arg = value_after(args, "--freq");
  if (!freq_arg.has_value()) {
    throw std::runtime_error("identify requires --freq");
  }
  const auto frequency = parse_u64_arg(*freq_arg, "frequency");
  const auto card = nevins::identify_frequency(frequency);
  std::cout << nevins::format_signal_card(card);
  return 0;
}

}  // namespace

int main(int argc, char** argv) {
  try {
    std::vector<std::string> args;
    for (int index = 1; index < argc; ++index) {
      args.emplace_back(argv[index]);
    }

    if (args.empty()) {
      print_usage();
      return 2;
    }

    if (args[0] == "--version") {
      std::cout << "Nevins Port " << nevins::kVersion << '\n';
      std::cout << nevins::kTagline << '\n';
      std::cout << "Receive-only radio operations console\n";
      return 0;
    }

    if (args[0] == "probe") {
      if (!has_arg(args, "--mock")) {
        std::cerr << "Real RTL-SDR probing is scaffolded; use --mock for this pass.\n";
        return 2;
      }
      return probe_mock();
    }

    if (args[0] == "survey") {
      if (!has_arg(args, "--mock")) {
        std::cerr << "Survey requires --mock unless real RTL-SDR mode is explicitly enabled later.\n";
        return 2;
      }
      return survey_mock(args);
    }

    if (args[0] == "identify") {
      if (!has_arg(args, "--mock")) {
        std::cerr << "Identify requires --mock for this pass.\n";
        return 2;
      }
      return identify_mock(args);
    }

    if (args[0] == "explain") {
      if (args.size() < 2U) {
        throw std::runtime_error("explain requires a capture pack path");
      }
      std::cout << nevins::explain_capture_pack(args[1]);
      return 0;
    }

    if (args[0] == "replay") {
      if (args.size() < 2U) {
        throw std::runtime_error("replay requires a capture pack path");
      }
      std::cout << nevins::replay_capture_pack(args[1]);
      return 0;
    }

    print_usage();
    return 2;
  } catch (const std::exception& error) {
    std::cerr << "nevins: " << error.what() << '\n';
    return 1;
  }
}
