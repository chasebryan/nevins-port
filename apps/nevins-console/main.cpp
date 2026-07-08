#include "nevins/signal_card.h"
#include "nevins/device.h"
#include "nevins/version.h"

#include <GLFW/glfw3.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <limits>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

namespace {

struct Color {
  float r;
  float g;
  float b;
};

struct Rect {
  float x;
  float y;
  float w;
  float h;
};

enum class ConsoleMode {
  Mock,
  RtlSdr
};

struct DashboardData {
  ConsoleMode mode = ConsoleMode::Mock;
  nevins::CaptureSettings settings;
  nevins::DeviceStatus status;
  nevins::SignalCard card;
  nevins::SpectrumSummary summary;
  std::size_t iq_bytes = 0;
  std::vector<double> spectrum_bins;
  std::vector<std::vector<double>> waterfall_rows;
  std::string event_line = "MOCK SPECTRUM FRAME UPDATED";
  std::string second_event_line = "SIGNAL CARD REFRESHED";
};

using Glyph = std::array<const char*, 7>;

Glyph glyph_for(char raw) {
  const auto ch = static_cast<char>(std::toupper(static_cast<unsigned char>(raw)));
  switch (ch) {
    case 'A':
      return {"01110", "10001", "10001", "11111", "10001", "10001", "10001"};
    case 'B':
      return {"11110", "10001", "10001", "11110", "10001", "10001", "11110"};
    case 'C':
      return {"01111", "10000", "10000", "10000", "10000", "10000", "01111"};
    case 'D':
      return {"11110", "10001", "10001", "10001", "10001", "10001", "11110"};
    case 'E':
      return {"11111", "10000", "10000", "11110", "10000", "10000", "11111"};
    case 'F':
      return {"11111", "10000", "10000", "11110", "10000", "10000", "10000"};
    case 'G':
      return {"01111", "10000", "10000", "10011", "10001", "10001", "01111"};
    case 'H':
      return {"10001", "10001", "10001", "11111", "10001", "10001", "10001"};
    case 'I':
      return {"11111", "00100", "00100", "00100", "00100", "00100", "11111"};
    case 'J':
      return {"11111", "00001", "00001", "00001", "00001", "10001", "01110"};
    case 'K':
      return {"10001", "10010", "10100", "11000", "10100", "10010", "10001"};
    case 'L':
      return {"10000", "10000", "10000", "10000", "10000", "10000", "11111"};
    case 'M':
      return {"10001", "11011", "10101", "10101", "10001", "10001", "10001"};
    case 'N':
      return {"10001", "11001", "10101", "10011", "10001", "10001", "10001"};
    case 'O':
      return {"01110", "10001", "10001", "10001", "10001", "10001", "01110"};
    case 'P':
      return {"11110", "10001", "10001", "11110", "10000", "10000", "10000"};
    case 'Q':
      return {"01110", "10001", "10001", "10001", "10101", "10010", "01101"};
    case 'R':
      return {"11110", "10001", "10001", "11110", "10100", "10010", "10001"};
    case 'S':
      return {"01111", "10000", "10000", "01110", "00001", "00001", "11110"};
    case 'T':
      return {"11111", "00100", "00100", "00100", "00100", "00100", "00100"};
    case 'U':
      return {"10001", "10001", "10001", "10001", "10001", "10001", "01110"};
    case 'V':
      return {"10001", "10001", "10001", "10001", "10001", "01010", "00100"};
    case 'W':
      return {"10001", "10001", "10001", "10101", "10101", "11011", "10001"};
    case 'X':
      return {"10001", "10001", "01010", "00100", "01010", "10001", "10001"};
    case 'Y':
      return {"10001", "10001", "01010", "00100", "00100", "00100", "00100"};
    case 'Z':
      return {"11111", "00001", "00010", "00100", "01000", "10000", "11111"};
    case '0':
      return {"01110", "10001", "10011", "10101", "11001", "10001", "01110"};
    case '1':
      return {"00100", "01100", "00100", "00100", "00100", "00100", "01110"};
    case '2':
      return {"01110", "10001", "00001", "00010", "00100", "01000", "11111"};
    case '3':
      return {"11110", "00001", "00001", "01110", "00001", "00001", "11110"};
    case '4':
      return {"00010", "00110", "01010", "10010", "11111", "00010", "00010"};
    case '5':
      return {"11111", "10000", "10000", "11110", "00001", "00001", "11110"};
    case '6':
      return {"01110", "10000", "10000", "11110", "10001", "10001", "01110"};
    case '7':
      return {"11111", "00001", "00010", "00100", "01000", "01000", "01000"};
    case '8':
      return {"01110", "10001", "10001", "01110", "10001", "10001", "01110"};
    case '9':
      return {"01110", "10001", "10001", "01111", "00001", "00001", "01110"};
    case ':':
      return {"00000", "00100", "00100", "00000", "00100", "00100", "00000"};
    case '.':
      return {"00000", "00000", "00000", "00000", "00000", "01100", "01100"};
    case ',':
      return {"00000", "00000", "00000", "00000", "01100", "00100", "01000"};
    case '-':
      return {"00000", "00000", "00000", "11111", "00000", "00000", "00000"};
    case '/':
      return {"00001", "00010", "00010", "00100", "01000", "01000", "10000"};
    case '|':
      return {"00100", "00100", "00100", "00100", "00100", "00100", "00100"};
    case '%':
      return {"11001", "11010", "00010", "00100", "01000", "01011", "10011"};
    case '+':
      return {"00000", "00100", "00100", "11111", "00100", "00100", "00000"};
    case '=':
      return {"00000", "00000", "11111", "00000", "11111", "00000", "00000"};
    case '_':
      return {"00000", "00000", "00000", "00000", "00000", "00000", "11111"};
    case '(':
      return {"00010", "00100", "01000", "01000", "01000", "00100", "00010"};
    case ')':
      return {"01000", "00100", "00010", "00010", "00010", "00100", "01000"};
    default:
      return {"00000", "00000", "00000", "00000", "00000", "00000", "00000"};
  }
}

std::vector<double> mock_bins(int frame, int count) {
  std::vector<double> bins;
  bins.reserve(static_cast<std::size_t>(count));
  for (int index = 0; index < count; ++index) {
    const double carrier = std::exp(-std::pow((static_cast<double>(index) - 26.0 - static_cast<double>(frame % 12)) / 5.0, 2.0));
    const double shoulder = 0.35 * std::exp(-std::pow((static_cast<double>(index) - 48.0) / 8.0, 2.0));
    const double floor = 0.10 + 0.04 * std::sin(static_cast<double>(index + frame) * 0.35);
    bins.push_back(std::clamp(floor + carrier + shoulder, 0.0, 1.0));
  }
  return bins;
}

std::string bar(double value, int width) {
  const auto filled = static_cast<int>(std::clamp(value, 0.0, 1.0) * static_cast<double>(width));
  std::string out;
  out.reserve(static_cast<std::size_t>(width));
  for (int index = 0; index < width; ++index) {
    out.push_back(index < filled ? '#' : '.');
  }
  return out;
}

std::vector<double> iq_frequency_bins(const std::vector<std::uint8_t>& iq,
                                      int count,
                                      std::size_t first_sample,
                                      std::size_t requested_samples) {
  std::vector<double> bins(static_cast<std::size_t>(count), 0.0);
  if (iq.size() < 2U || count <= 0) {
    return bins;
  }

  const std::size_t sample_count = iq.size() / 2U;
  if (first_sample >= sample_count) {
    return bins;
  }

  const std::size_t available = std::min(requested_samples, sample_count - first_sample);
  if (available == 0U) {
    return bins;
  }

  const std::size_t window_samples = std::min<std::size_t>(available, 2048U);
  constexpr double kPi = 3.14159265358979323846;
  std::vector<double> powers(static_cast<std::size_t>(count), 0.0);

  for (int bin = 0; bin < count; ++bin) {
    const double normalized_frequency =
        -0.5 + (static_cast<double>(bin) + 0.5) / static_cast<double>(std::max(count, 1));
    double real = 0.0;
    double imag = 0.0;
    for (std::size_t n = 0; n < window_samples; ++n) {
      const std::size_t sample = first_sample + n;
      const double i = (static_cast<double>(iq[2U * sample]) - 127.5) / 127.5;
      const double q = (static_cast<double>(iq[2U * sample + 1U]) - 127.5) / 127.5;
      const double window = 0.5 - 0.5 * std::cos((2.0 * kPi * static_cast<double>(n)) /
                                                  static_cast<double>(std::max<std::size_t>(1U, window_samples - 1U)));
      const double angle = -2.0 * kPi * normalized_frequency * static_cast<double>(n);
      const double cosine = std::cos(angle);
      const double sine = std::sin(angle);
      real += window * ((i * cosine) - (q * sine));
      imag += window * ((i * sine) + (q * cosine));
    }
    powers[static_cast<std::size_t>(bin)] = (real * real + imag * imag) /
                                            static_cast<double>(std::max<std::size_t>(1U, window_samples));
  }

  double min_db = std::numeric_limits<double>::max();
  double max_db = -std::numeric_limits<double>::max();
  for (double& value : powers) {
    value = 10.0 * std::log10(std::max(value, 1.0e-12));
    min_db = std::min(min_db, value);
    max_db = std::max(max_db, value);
  }

  const double range = std::max(1.0e-9, max_db - min_db);
  for (std::size_t index = 0; index < powers.size(); ++index) {
    bins[index] = std::clamp((powers[index] - min_db) / range, 0.0, 1.0);
  }
  return bins;
}

std::vector<double> iq_bins(const std::vector<std::uint8_t>& iq, int count) {
  return iq_frequency_bins(iq, count, 0U, iq.size() / 2U);
}

std::vector<std::vector<double>> waterfall_from_iq(const std::vector<std::uint8_t>& iq, int columns, int rows) {
  std::vector<std::vector<double>> waterfall;
  waterfall.reserve(static_cast<std::size_t>(rows));
  const std::size_t sample_count = iq.size() / 2U;
  if (sample_count == 0U) {
    return waterfall;
  }

  const std::size_t step = std::max<std::size_t>(1U, sample_count / static_cast<std::size_t>(std::max(rows, 1)));
  const std::size_t window = std::max<std::size_t>(256U, step);
  for (int row = 0; row < rows; ++row) {
    const std::size_t first = std::min(sample_count - 1U, static_cast<std::size_t>(row) * step);
    waterfall.push_back(iq_frequency_bins(iq, columns, first, window));
  }
  return waterfall;
}

std::vector<std::vector<double>> waterfall_from_bins(const std::vector<double>& current, int frame) {
  constexpr int rows = 48;
  std::vector<std::vector<double>> waterfall;
  waterfall.reserve(rows);
  if (current.empty()) {
    for (int row = 0; row < rows; ++row) {
      waterfall.push_back(mock_bins(frame - row, 96));
    }
    return waterfall;
  }

  for (int row = 0; row < rows; ++row) {
    std::vector<double> shifted = current;
    for (std::size_t column = 0; column < shifted.size(); ++column) {
      const double ripple = 0.04 * std::sin(static_cast<double>(frame + row + static_cast<int>(column)) * 0.23);
      shifted[column] = std::clamp(shifted[column] * (1.0 - static_cast<double>(row) * 0.010) + ripple, 0.0, 1.0);
    }
    waterfall.push_back(std::move(shifted));
  }
  return waterfall;
}

nevins::SignalCard card_from_summary(const nevins::SpectrumSummary& summary, const nevins::CaptureSettings& settings) {
  nevins::SignalObservation observation;
  observation.center_frequency_hz = settings.center_frequency_hz;
  observation.sample_rate_hz = settings.sample_rate_hz;
  observation.duration_ms = settings.duration_ms;
  observation.spectrum = summary;
  return nevins::classify_signal(observation);
}

void set_color(Color color) {
  glColor3f(color.r, color.g, color.b);
}

void fill_rect(Rect rect, Color color) {
  set_color(color);
  glBegin(GL_QUADS);
  glVertex2f(rect.x, rect.y);
  glVertex2f(rect.x + rect.w, rect.y);
  glVertex2f(rect.x + rect.w, rect.y + rect.h);
  glVertex2f(rect.x, rect.y + rect.h);
  glEnd();
}

void draw_text(float x, float y, const std::string& text, float scale, Color color) {
  set_color(color);
  float cursor = x;
  for (const char raw : text) {
    if (raw == ' ') {
      cursor += 4.0F * scale;
      continue;
    }
    const auto glyph = glyph_for(raw);
    for (std::size_t row = 0; row < glyph.size(); ++row) {
      for (std::size_t column = 0; column < 5U; ++column) {
        if (glyph[row][column] == '1') {
          fill_rect(Rect{
                        cursor + static_cast<float>(column) * scale,
                        y + static_cast<float>(row) * scale,
                        scale,
                        scale,
                    },
                    color);
        }
      }
    }
    cursor += 6.0F * scale;
  }
}

float text_width(const std::string& text, float scale) {
  float width = 0.0F;
  for (const char raw : text) {
    width += (raw == ' ' ? 4.0F : 6.0F) * scale;
  }
  return width;
}

void draw_text_fit(float x, float y, const std::string& text, float scale, float max_width, Color color) {
  const float width = text_width(text, scale);
  if (width <= max_width) {
    draw_text(x, y, text, scale, color);
    return;
  }
  const float fitted = std::max(0.75F, scale * (max_width / std::max(width, 1.0F)));
  draw_text(x, y, text, fitted, color);
}

void line_rect(Rect rect, Color color) {
  set_color(color);
  glBegin(GL_LINE_LOOP);
  glVertex2f(rect.x, rect.y);
  glVertex2f(rect.x + rect.w, rect.y);
  glVertex2f(rect.x + rect.w, rect.y + rect.h);
  glVertex2f(rect.x, rect.y + rect.h);
  glEnd();
}

void draw_line(float x0, float y0, float x1, float y1, Color color) {
  set_color(color);
  glBegin(GL_LINES);
  glVertex2f(x0, y0);
  glVertex2f(x1, y1);
  glEnd();
}

void draw_panel(Rect rect, Color fill) {
  fill_rect(rect, fill);
  line_rect(rect, Color{0.28F, 0.33F, 0.35F});
}

void draw_panel_title(Rect rect, const std::string& title) {
  draw_text(rect.x + 12.0F, rect.y + 12.0F, title, 2.0F, Color{0.78F, 0.85F, 0.83F});
  draw_line(rect.x + 12.0F, rect.y + 34.0F, rect.x + rect.w - 12.0F, rect.y + 34.0F, Color{0.18F, 0.22F, 0.23F});
}

std::string format_db(double value) {
  std::ostringstream output;
  output << std::fixed << std::setprecision(1) << value << " DB";
  return output.str();
}

std::string format_hz(std::uint32_t value) {
  if (value >= 1000000U) {
    std::ostringstream output;
    output << std::fixed << std::setprecision(2) << (static_cast<double>(value) / 1000000.0) << " MHZ";
    return output.str();
  }
  if (value >= 1000U) {
    std::ostringstream output;
    output << std::fixed << std::setprecision(1) << (static_cast<double>(value) / 1000.0) << " KHZ";
    return output.str();
  }
  return std::to_string(value) + " HZ";
}

void draw_status_badge(float x, float y, const std::string& text, Color accent) {
  fill_rect(Rect{x, y, 10.0F, 10.0F}, accent);
  fill_rect(Rect{x + 14.0F, y + 4.0F, 42.0F, 2.0F}, Color{0.20F, 0.32F, 0.34F});
  draw_text(x + 62.0F, y - 2.0F, text, 1.1F, Color{0.66F, 0.76F, 0.72F});
}

void draw_spectrum(Rect rect, const std::vector<double>& bins, const std::string& footer) {
  draw_panel(rect, Color{0.055F, 0.070F, 0.075F});
  draw_panel_title(rect, "SPECTRUM VIEW");
  const float step = rect.w / static_cast<float>(std::max<std::size_t>(1U, bins.size() - 1U));
  set_color(Color{0.22F, 0.78F, 0.70F});
  glBegin(GL_LINE_STRIP);
  for (std::size_t index = 0; index < bins.size(); ++index) {
    const float x = rect.x + step * static_cast<float>(index);
    const float y = rect.y + 40.0F + (rect.h - 52.0F) * (0.12F + 0.78F * static_cast<float>(bins[index]));
    glVertex2f(x, y);
  }
  glEnd();
  draw_text_fit(rect.x + 14.0F, rect.y + rect.h - 18.0F, footer, 1.4F, rect.w - 28.0F, Color{0.50F, 0.61F, 0.61F});
}

void draw_waterfall(Rect rect, const std::vector<std::vector<double>>& rows_data) {
  draw_panel(rect, Color{0.050F, 0.058F, 0.064F});
  draw_panel_title(rect, "WATERFALL VIEW");
  const int rows = static_cast<int>(rows_data.size());
  const int columns = rows > 0 ? static_cast<int>(rows_data.front().size()) : 96;
  const float cell_w = rect.w / static_cast<float>(columns);
  const float plot_y = rect.y + 42.0F;
  const float plot_h = rect.h - 54.0F;
  const float cell_h = plot_h / static_cast<float>(std::max(rows, 1));
  for (int row = 0; row < rows; ++row) {
    for (int column = 0; column < columns; ++column) {
      const auto value = static_cast<float>(rows_data[static_cast<std::size_t>(row)][static_cast<std::size_t>(column)]);
      const Color color{
          0.08F + 0.55F * value,
          0.10F + 0.35F * value,
          0.14F + 0.08F * value,
      };
      fill_rect(Rect{
                    rect.x + static_cast<float>(column) * cell_w,
                    plot_y + static_cast<float>(row) * cell_h,
                    cell_w,
                    cell_h,
                },
                color);
    }
  }
  line_rect(rect, Color{0.28F, 0.33F, 0.35F});
}

void draw_card_panel(Rect rect, const nevins::SignalCard& card, const nevins::SpectrumSummary& summary) {
  draw_panel(rect, Color{0.075F, 0.082F, 0.085F});
  draw_panel_title(rect, "SIGNAL CARDS");
  draw_text(rect.x + 14.0F, rect.y + 52.0F, nevins::format_frequency_mhz(card.frequency_hz), 1.85F, Color{0.82F, 0.88F, 0.84F});
  draw_text_fit(rect.x + 14.0F, rect.y + 80.0F, card.type_guess, 1.15F, rect.w - 28.0F, Color{0.58F, 0.72F, 0.68F});
  draw_text(rect.x + 14.0F, rect.y + 102.0F, std::string("CONFIDENCE ") + std::to_string(card.confidence) + "%", 1.15F, Color{0.58F, 0.72F, 0.68F});

  const float meter = std::clamp(static_cast<float>(card.confidence) / 100.0F, 0.0F, 1.0F);
  const Rect meter_track{rect.x + 14.0F, rect.y + 124.0F, rect.w - 28.0F, 10.0F};
  fill_rect(meter_track, Color{0.12F, 0.14F, 0.15F});
  fill_rect(Rect{meter_track.x, meter_track.y, meter_track.w * meter, meter_track.h},
            Color{0.20F, 0.67F, 0.54F});

  draw_line(rect.x + 14.0F, rect.y + 154.0F, rect.x + rect.w - 14.0F, rect.y + 154.0F, Color{0.30F, 0.34F, 0.35F});
  draw_text(rect.x + 14.0F, rect.y + 174.0F, std::string("BANDWIDTH ") + format_hz(card.bandwidth_estimate_hz), 1.1F, Color{0.70F, 0.78F, 0.76F});
  draw_text(rect.x + 14.0F, rect.y + 198.0F, std::string("PEAK ") + format_db(summary.peak_power_db), 1.1F, Color{0.66F, 0.74F, 0.72F});
  draw_text(rect.x + 14.0F, rect.y + 222.0F, std::string("NOISE ") + format_db(summary.noise_floor_db), 1.1F, Color{0.66F, 0.74F, 0.72F});
  draw_text(rect.x + 14.0F, rect.y + 246.0F, std::string("PEAK BIN ") + nevins::format_frequency_mhz(summary.peak_frequency_hz), 1.0F, Color{0.58F, 0.70F, 0.74F});
}

void draw_status_panel(Rect rect, const DashboardData& data, int frame) {
  draw_panel(rect, Color{0.070F, 0.078F, 0.080F});
  draw_panel_title(rect, "DEVICE STATUS");
  draw_text_fit(rect.x + 14.0F, rect.y + 50.0F, data.status.name, 1.15F, rect.w - 158.0F, Color{0.62F, 0.74F, 0.72F});
  draw_text(rect.x + 14.0F,
            rect.y + 72.0F,
            std::string("CENTER ") + nevins::format_frequency_mhz(data.settings.center_frequency_hz),
            1.15F,
            Color{0.62F, 0.74F, 0.72F});
  draw_text(rect.x + 14.0F,
            rect.y + 94.0F,
            std::string("SAMPLE ") + std::to_string(data.settings.sample_rate_hz) + " HZ",
            1.15F,
            Color{0.62F, 0.74F, 0.72F});
  const float pulse = 0.35F + 0.25F * std::sin(static_cast<float>(frame) * 0.08F);
  draw_status_badge(rect.x + rect.w - 132.0F,
                    rect.y + 48.0F,
                    data.mode == ConsoleMode::RtlSdr ? "RTL OK" : "MOCK OK",
                    Color{0.10F, 0.56F + pulse, 0.28F});
}

void draw_metrics_panel(Rect rect, const DashboardData& data) {
  draw_panel(rect, Color{0.065F, 0.080F, 0.075F});
  draw_panel_title(rect, "RECEIVER METRICS");
  const std::string gain =
      data.settings.gain_db_tenths == 0
          ? "AUTO"
          : std::to_string(data.settings.gain_db_tenths / 10) + "." +
                std::to_string(std::abs(data.settings.gain_db_tenths % 10)) + " DB";
  draw_text_fit(rect.x + 14.0F,
                rect.y + 52.0F,
                std::string("SAMPLE ") + std::to_string(data.settings.sample_rate_hz) + " HZ",
                1.15F,
                rect.w - 28.0F,
                Color{0.68F, 0.80F, 0.76F});
  draw_text_fit(rect.x + 14.0F,
                rect.y + 74.0F,
                std::string("WINDOW ") + std::to_string(data.settings.duration_ms) + " MS",
                1.15F,
                rect.w - 28.0F,
                Color{0.68F, 0.80F, 0.76F});
  draw_text_fit(rect.x + 14.0F, rect.y + 96.0F, std::string("GAIN ") + gain, 1.15F, rect.w - 28.0F, Color{0.68F, 0.80F, 0.76F});
  draw_text_fit(rect.x + 14.0F,
                rect.y + 118.0F,
                std::string("IQ BYTES ") + std::to_string(data.iq_bytes),
                1.15F,
                rect.w - 28.0F,
                Color{0.62F, 0.72F, 0.76F});
  draw_text_fit(rect.x + 14.0F,
                rect.y + 148.0F,
                std::string("PEAK ") + format_db(data.summary.peak_power_db),
                1.15F,
                rect.w - 28.0F,
                Color{0.70F, 0.78F, 0.76F});
  draw_text_fit(rect.x + 14.0F,
                rect.y + 170.0F,
                std::string("FLOOR ") + format_db(data.summary.noise_floor_db),
                1.15F,
                rect.w - 28.0F,
                Color{0.70F, 0.78F, 0.76F});
  draw_text_fit(rect.x + 14.0F,
                rect.y + 192.0F,
                std::string("BW ") + format_hz(data.summary.bandwidth_estimate_hz),
                1.15F,
                rect.w - 28.0F,
                Color{0.70F, 0.78F, 0.76F});
  draw_text_fit(rect.x + 14.0F,
                rect.y + 214.0F,
                std::string("PEAK BIN ") + nevins::format_frequency_mhz(data.summary.peak_frequency_hz),
                1.05F,
                rect.w - 28.0F,
                Color{0.58F, 0.70F, 0.74F});
}

void render_dashboard(int width, int height, int frame, const DashboardData& data) {
  glViewport(0, 0, width, height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0.0, static_cast<double>(width), static_cast<double>(height), 0.0, -1.0, 1.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glClearColor(0.035F, 0.043F, 0.046F, 1.0F);
  glClear(GL_COLOR_BUFFER_BIT);

  const float w = static_cast<float>(width);
  const float h = static_cast<float>(height);
  const float gap = 12.0F;
  const Rect device{gap, gap, w * 0.28F - gap * 1.5F, h * 0.18F};
  const Rect metrics{gap, device.y + device.h + gap, device.w, h * 0.28F};
  const Rect library{gap, metrics.y + metrics.h + gap, device.w, h - metrics.y - metrics.h - gap * 2.0F};
  const Rect spectrum{device.x + device.w + gap, gap, w * 0.44F, h * 0.34F};
  const Rect waterfall{spectrum.x, spectrum.y + spectrum.h + gap, spectrum.w, h - spectrum.h - gap * 3.0F};
  const Rect card{spectrum.x + spectrum.w + gap, gap, w - spectrum.x - spectrum.w - gap * 2.0F, h * 0.38F};
  const Rect log{card.x, card.y + card.h + gap, card.w, h - card.h - gap * 3.0F};

  draw_status_panel(device, data, frame);
  draw_metrics_panel(metrics, data);
  draw_panel(library, Color{0.065F, 0.070F, 0.078F});
  draw_panel_title(library, "CAPTURE LIBRARY");
  draw_text(library.x + 14.0F,
            library.y + 52.0F,
            data.mode == ConsoleMode::RtlSdr ? "RTL-SDR LIVE SNAPSHOT" : "MOCK-SURVEY.NVCAP",
            1.25F,
            Color{0.62F, 0.72F, 0.76F});
  draw_text(library.x + 14.0F, library.y + 74.0F, "MANIFEST.NV1 / IQ.U8", 1.25F, Color{0.62F, 0.72F, 0.76F});
  draw_text(library.x + 14.0F, library.y + 96.0F, "SIGNAL-CARD.MD", 1.25F, Color{0.62F, 0.72F, 0.76F});
  draw_spectrum(spectrum,
                data.spectrum_bins.empty() ? mock_bins(frame, 96) : data.spectrum_bins,
                std::string("CENTER ") + nevins::format_frequency_mhz(data.settings.center_frequency_hz) +
                    "  PEAK " + nevins::format_frequency_mhz(data.summary.peak_frequency_hz) +
                    "  BW " + format_hz(data.summary.bandwidth_estimate_hz));
  draw_waterfall(waterfall, data.waterfall_rows.empty() ? waterfall_from_bins(mock_bins(frame, 96), frame) : data.waterfall_rows);
  draw_card_panel(card, data.card, data.summary);
  draw_panel(log, Color{0.060F, 0.066F, 0.068F});
  draw_panel_title(log, "EVENT LOG");
  draw_text_fit(log.x + 14.0F, log.y + 52.0F, data.event_line, 1.25F, log.w - 28.0F, Color{0.66F, 0.74F, 0.72F});
  draw_text_fit(log.x + 14.0F, log.y + 74.0F, data.second_event_line, 1.25F, log.w - 28.0F, Color{0.66F, 0.74F, 0.72F});
  draw_text_fit(log.x + 14.0F,
                log.y + 96.0F,
                std::string("PEAK TRACK ") + nevins::format_frequency_mhz(data.summary.peak_frequency_hz),
                1.25F,
                log.w - 28.0F,
                Color{0.78F, 0.72F, 0.58F});
}

std::optional<std::string> value_after(int argc, char** argv, const std::string& key) {
  for (int index = 1; index + 1 < argc; ++index) {
    if (argv[index] == key) {
      return argv[index + 1];
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

std::int32_t parse_gain_db_tenths(const std::string& value) {
  std::size_t consumed = 0;
  const double parsed = std::stod(value, &consumed);
  if (consumed != value.size()) {
    throw std::runtime_error("invalid gain value");
  }
  return static_cast<std::int32_t>(std::lround(parsed * 10.0));
}

DashboardData make_mock_dashboard_data(const nevins::CaptureSettings& settings, int frame) {
  (void)frame;
  DashboardData data;
  data.mode = ConsoleMode::Mock;
  nevins::MockSdrSource source;
  data.status = source.status();
  data.settings = settings;
  const auto iq = source.read_iq(settings);
  data.iq_bytes = iq.size();
  data.summary = nevins::analyze_iq_power(iq, settings);
  data.spectrum_bins = iq_bins(iq, 96);
  data.waterfall_rows = waterfall_from_iq(iq, 96, 48);
  data.card = card_from_summary(data.summary, settings);
  data.event_line = "MOCK SPECTRUM FRAME UPDATED";
  data.second_event_line = "MOCK IQ WINDOW ANALYZED";
  return data;
}

DashboardData make_rtlsdr_dashboard_data(const nevins::CaptureSettings& settings, int frame) {
  (void)frame;
  DashboardData data;
  data.mode = ConsoleMode::RtlSdr;
  nevins::RtlSdrSource source;
  data.status = source.status();
  data.settings = settings;
  const auto iq = source.read_iq(settings);
  data.iq_bytes = iq.size();
  data.summary = nevins::analyze_iq_power(iq, settings);
  data.spectrum_bins = iq_bins(iq, 96);
  data.waterfall_rows = waterfall_from_iq(iq, 96, 48);
  data.card = card_from_summary(data.summary, settings);
  data.event_line = "RTL-SDR IQ SNAPSHOT UPDATED";
  data.second_event_line = "RTL-SDR WATERFALL WINDOW ANALYZED";
  return data;
}

void render_text_dashboard(const DashboardData& data) {
  const auto& card = data.card;
  for (int frame = 0; frame < 6; ++frame) {
    const auto bins = data.spectrum_bins.empty() ? mock_bins(frame, 64) : data.spectrum_bins;
    std::cout << "\x1b[2J\x1b[H";
    std::cout << nevins::kProjectName << " " << nevins::kVersion << " - " << nevins::kTagline << "\n";
    std::cout << "Native " << (data.mode == ConsoleMode::RtlSdr ? "RTL-SDR" : "mock") << " receiver dashboard.\n";
    std::cout << "\n== Device Status ==\n";
    std::cout << "Mode: " << data.status.mode << " | Receiver: " << data.status.name
              << " | Center: " << nevins::format_frequency_mhz(data.settings.center_frequency_hz)
              << " | Sample rate: " << data.settings.sample_rate_hz << " Hz | Runtime frame: " << frame << "\n";
    std::cout << "\n== Spectrum View ==\n";
    for (std::size_t index = 0; index < bins.size(); index += 8U) {
      std::cout << std::setw(2) << index << " ";
      for (std::size_t inner = index; inner < index + 8U && inner < bins.size(); ++inner) {
        std::cout << bar(bins[inner], 8) << ' ';
      }
      std::cout << '\n';
    }
    std::cout << "\n== Waterfall View ==\n";
    const auto waterfall = data.waterfall_rows.empty() ? waterfall_from_bins(mock_bins(frame, 56), frame) : data.waterfall_rows;
    for (int row = 0; row < 10 && row < static_cast<int>(waterfall.size()); ++row) {
      for (double value : waterfall[static_cast<std::size_t>(row)]) {
        std::cout << (value > 0.78 ? '#' : value > 0.45 ? '+' : value > 0.22 ? '-' : '.');
      }
      std::cout << '\n';
    }
    std::cout << "\n== Signal Cards ==\n";
    std::cout << "Frequency: " << nevins::format_frequency_mhz(card.frequency_hz) << " | Type: " << card.type_guess
              << " | Confidence: " << card.confidence << "%\n";
    std::cout << "Category: " << nv_signal_category_name(card.category) << " | Bandwidth: "
              << card.bandwidth_estimate_hz << " Hz | Peak: " << std::fixed << std::setprecision(1)
              << data.summary.peak_power_db << " dB\n";
    std::cout << "\n== Capture Library ==\n";
    std::cout << (data.mode == ConsoleMode::RtlSdr ? "rtl-sdr live snapshot" : "mock-survey.nvcap")
              << " | replayable IQ pack | manifest.nv1 | signal-card.md\n";
    std::cout << "\n== Receiver Metrics ==\n";
    std::cout << "IQ bytes: " << data.iq_bytes << " | Noise floor: " << std::fixed << std::setprecision(1)
              << data.summary.noise_floor_db << " dB | Peak bin: "
              << nevins::format_frequency_mhz(data.summary.peak_frequency_hz) << " | BW: "
              << data.summary.bandwidth_estimate_hz << " Hz\n";
    std::cout << "\n== Event Log ==\n";
    std::cout << "[" << data.status.mode << "] " << data.event_line << '\n';
    std::cout << "[" << data.status.mode << "] " << data.second_event_line << '\n';
    std::cout << "[" << data.status.mode << "] peak track " << nevins::format_frequency_mhz(data.summary.peak_frequency_hz)
              << '\n';
    std::this_thread::sleep_for(std::chrono::milliseconds(120));
  }
}

bool has_arg(int argc, char** argv, const std::string& arg) {
  for (int index = 1; index < argc; ++index) {
    if (argv[index] == arg) {
      return true;
    }
  }
  return false;
}

}  // namespace

int main(int argc, char** argv) {
  try {
    const bool requested_mock = has_arg(argc, argv, "--mock");
    const bool requested_rtlsdr = has_arg(argc, argv, "--rtlsdr");
    if (requested_mock && requested_rtlsdr) {
      std::cerr << "Choose either --mock or --rtlsdr, not both.\n";
      return 2;
    }

    const ConsoleMode mode = requested_rtlsdr ? ConsoleMode::RtlSdr : ConsoleMode::Mock;
    nevins::CaptureSettings settings;
    settings.center_frequency_hz = parse_u64_arg(value_after(argc, argv, "--freq").value_or("433920000"), "frequency");
    settings.sample_rate_hz = parse_u32_arg(value_after(argc, argv, "--rate").value_or("2048000"), "sample rate");
    settings.duration_ms = parse_u32_arg(value_after(argc, argv, "--window-ms").value_or("250"), "window");
    settings.gain_db_tenths = parse_gain_db_tenths(value_after(argc, argv, "--gain-db").value_or("0"));

    DashboardData data =
        mode == ConsoleMode::RtlSdr ? make_rtlsdr_dashboard_data(settings, 0) : make_mock_dashboard_data(settings, 0);

    if (has_arg(argc, argv, "--text")) {
      render_text_dashboard(data);
      return 0;
    }

    if (glfwInit() != GLFW_TRUE) {
      std::cerr << "GLFW initialization failed; falling back to terminal dashboard.\n";
      render_text_dashboard(data);
      return 0;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    GLFWwindow* window = glfwCreateWindow(
        1180,
        760,
        "Nevins Port - Device | Spectrum | Waterfall | Signal Cards | Capture Library | Metrics | Event Log",
        nullptr,
        nullptr);

    if (window == nullptr) {
      glfwTerminate();
      std::cerr << "GLFW window creation failed; falling back to terminal dashboard.\n";
      render_text_dashboard(data);
      return 0;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    int frame = 0;
    const bool smoke = has_arg(argc, argv, "--smoke");
    while (glfwWindowShouldClose(window) == GLFW_FALSE) {
      if (frame > 0 && frame % 120 == 0) {
        data = mode == ConsoleMode::RtlSdr ? make_rtlsdr_dashboard_data(settings, frame) : make_mock_dashboard_data(settings, frame);
      }

      int width = 0;
      int height = 0;
      glfwGetFramebufferSize(window, &width, &height);
      render_dashboard(std::max(width, 1), std::max(height, 1), frame, data);
      glfwSwapBuffers(window);
      glfwPollEvents();
      ++frame;
      if (smoke && frame >= 12) {
        break;
      }
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "nevins-console: " << error.what() << '\n';
    return 1;
  }
}
