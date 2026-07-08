#include "nevins/signal_card.h"
#include "nevins/version.h"

#include <GLFW/glfw3.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cctype>
#include <cstdint>
#include <iomanip>
#include <iostream>
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

void draw_spectrum(Rect rect, const std::vector<double>& bins) {
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
  draw_text(rect.x + 14.0F, rect.y + rect.h - 18.0F, "CENTER 433.920 MHZ", 1.4F, Color{0.50F, 0.61F, 0.61F});
}

void draw_waterfall(Rect rect, int frame) {
  draw_panel(rect, Color{0.050F, 0.058F, 0.064F});
  draw_panel_title(rect, "WATERFALL VIEW");
  constexpr int rows = 48;
  constexpr int columns = 96;
  const float cell_w = rect.w / static_cast<float>(columns);
  const float plot_y = rect.y + 42.0F;
  const float plot_h = rect.h - 54.0F;
  const float cell_h = plot_h / static_cast<float>(rows);
  for (int row = 0; row < rows; ++row) {
    const auto bins = mock_bins(frame - row, columns);
    for (int column = 0; column < columns; ++column) {
      const auto value = static_cast<float>(bins[static_cast<std::size_t>(column)]);
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

void draw_card_panel(Rect rect, int confidence) {
  draw_panel(rect, Color{0.075F, 0.082F, 0.085F});
  draw_panel_title(rect, "SIGNAL CARDS");
  draw_text(rect.x + 14.0F, rect.y + 52.0F, "433.920 MHZ", 2.0F, Color{0.82F, 0.88F, 0.84F});
  draw_text(rect.x + 14.0F, rect.y + 78.0F, "BURSTY OOK/ASK-LIKE ISM", 1.35F, Color{0.58F, 0.72F, 0.68F});
  draw_text(rect.x + 14.0F, rect.y + 100.0F, "CONFIDENCE 68%", 1.35F, Color{0.58F, 0.72F, 0.68F});
  draw_text(rect.x + 14.0F, rect.y + 150.0F, "SAFE: RECORD IQ / REPLAY / EXPLAIN", 1.2F, Color{0.66F, 0.74F, 0.72F});
  draw_text(rect.x + 14.0F, rect.y + 172.0F, "DO NOT INFER OWNER OR CONTENT", 1.2F, Color{0.78F, 0.62F, 0.52F});
  const float meter = std::clamp(static_cast<float>(confidence) / 100.0F, 0.0F, 1.0F);
  fill_rect(Rect{rect.x + 0.04F * rect.w, rect.y + 0.58F * rect.h, rect.w * 0.92F, rect.h * 0.08F},
            Color{0.12F, 0.14F, 0.15F});
  fill_rect(Rect{rect.x + 0.04F * rect.w, rect.y + 0.58F * rect.h, rect.w * 0.92F * meter, rect.h * 0.08F},
            Color{0.20F, 0.67F, 0.54F});
  draw_line(rect.x + 0.04F * rect.w, rect.y + 0.34F * rect.h, rect.x + 0.96F * rect.w, rect.y + 0.34F * rect.h,
            Color{0.30F, 0.34F, 0.35F});
}

void draw_status_panel(Rect rect, int frame) {
  draw_panel(rect, Color{0.070F, 0.078F, 0.080F});
  draw_panel_title(rect, "DEVICE STATUS");
  draw_text(rect.x + 14.0F, rect.y + 52.0F, "MOCK RTL-SDR SOURCE", 1.35F, Color{0.62F, 0.74F, 0.72F});
  draw_text(rect.x + 14.0F, rect.y + 76.0F, "RECEIVE-ONLY OK", 1.35F, Color{0.56F, 0.82F, 0.62F});
  const float pulse = 0.35F + 0.25F * std::sin(static_cast<float>(frame) * 0.08F);
  fill_rect(Rect{rect.x + 0.04F * rect.w, rect.y + 0.34F * rect.h, rect.w * 0.14F, rect.h * 0.32F},
            Color{0.10F, 0.56F + pulse, 0.28F});
  fill_rect(Rect{rect.x + 0.23F * rect.w, rect.y + 0.40F * rect.h, rect.w * 0.68F, rect.h * 0.10F},
            Color{0.18F, 0.28F, 0.31F});
}

void render_dashboard(int width, int height, int frame) {
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
  const Rect policy{gap, device.y + device.h + gap, device.w, h * 0.20F};
  const Rect library{gap, policy.y + policy.h + gap, device.w, h - policy.y - policy.h - gap * 2.0F};
  const Rect spectrum{device.x + device.w + gap, gap, w * 0.44F, h * 0.34F};
  const Rect waterfall{spectrum.x, spectrum.y + spectrum.h + gap, spectrum.w, h - spectrum.h - gap * 3.0F};
  const Rect card{spectrum.x + spectrum.w + gap, gap, w - spectrum.x - spectrum.w - gap * 2.0F, h * 0.38F};
  const Rect log{card.x, card.y + card.h + gap, card.w, h - card.h - gap * 3.0F};

  draw_status_panel(device, frame);
  draw_panel(policy, Color{0.065F, 0.080F, 0.075F});
  draw_panel_title(policy, "POLICY / SAFETY");
  draw_text(policy.x + 14.0F, policy.y + 52.0F, "NO TRANSMIT", 1.25F, Color{0.68F, 0.82F, 0.70F});
  draw_text(policy.x + 14.0F, policy.y + 74.0F, "NO JAMMING OR SPOOFING", 1.25F, Color{0.68F, 0.82F, 0.70F});
  draw_text(policy.x + 14.0F, policy.y + 96.0F, "UNKNOWN: EXPLAIN-ONLY", 1.25F, Color{0.78F, 0.72F, 0.55F});
  draw_panel(library, Color{0.065F, 0.070F, 0.078F});
  draw_panel_title(library, "CAPTURE LIBRARY");
  draw_text(library.x + 14.0F, library.y + 52.0F, "MOCK-SURVEY.NVCAP", 1.25F, Color{0.62F, 0.72F, 0.76F});
  draw_text(library.x + 14.0F, library.y + 74.0F, "MANIFEST.NV1 / IQ.U8", 1.25F, Color{0.62F, 0.72F, 0.76F});
  draw_text(library.x + 14.0F, library.y + 96.0F, "SIGNAL-CARD.MD", 1.25F, Color{0.62F, 0.72F, 0.76F});
  draw_spectrum(spectrum, mock_bins(frame, 96));
  draw_waterfall(waterfall, frame);
  draw_card_panel(card, 68);
  draw_panel(log, Color{0.060F, 0.066F, 0.068F});
  draw_panel_title(log, "EVENT LOG");
  draw_text(log.x + 14.0F, log.y + 52.0F, "MOCK SPECTRUM FRAME UPDATED", 1.25F, Color{0.66F, 0.74F, 0.72F});
  draw_text(log.x + 14.0F, log.y + 74.0F, "CONSERVATIVE ISM CARD REFRESHED", 1.25F, Color{0.66F, 0.74F, 0.72F});
  draw_text(log.x + 14.0F, log.y + 96.0F, "RESTRICTED DECODE ROUTE BLOCKED", 1.25F, Color{0.78F, 0.62F, 0.52F});
}

void render_text_dashboard() {
  const auto card = nevins::identify_frequency(433920000, 1000);
  for (int frame = 0; frame < 6; ++frame) {
    const auto bins = mock_bins(frame, 64);
    std::cout << "\x1b[2J\x1b[H";
    std::cout << nevins::kProjectName << " " << nevins::kVersion << " - " << nevins::kTagline << "\n";
    std::cout << "Native mock dashboard. Receive-only. No content decoding.\n";
    std::cout << "\n== Device Status ==\n";
    std::cout << "Mode: mock receiver | Center: 433.920 MHz | Sample rate: 2048000 Hz | Runtime frame: " << frame
              << "\n";
    std::cout << "\n== Spectrum View ==\n";
    for (std::size_t index = 0; index < bins.size(); index += 8U) {
      std::cout << std::setw(2) << index << " ";
      for (std::size_t inner = index; inner < index + 8U && inner < bins.size(); ++inner) {
        std::cout << bar(bins[inner], 8) << ' ';
      }
      std::cout << '\n';
    }
    std::cout << "\n== Waterfall View ==\n";
    for (int row = 0; row < 10; ++row) {
      for (double value : mock_bins(frame - row, 56)) {
        std::cout << (value > 0.78 ? '#' : value > 0.45 ? '+' : value > 0.22 ? '-' : '.');
      }
      std::cout << '\n';
    }
    std::cout << "\n== Signal Cards ==\n";
    std::cout << "Frequency: " << nevins::format_frequency_mhz(card.frequency_hz) << " | Type: " << card.type_guess
              << " | Confidence: " << card.confidence << "%\n";
    std::cout << "Category: " << nv_signal_category_name(card.category) << " | Policy: " << card.policy << '\n';
    std::cout << "\n== Capture Library ==\n";
    std::cout << "mock-survey.nvcap | replayable IQ pack | manifest.nv1 | signal-card.md\n";
    std::cout << "\n== Policy/Safety Status ==\n";
    std::cout << "Receive-only: OK | Restricted decode route: blocked | Unknown route: explain-only\n";
    std::cout << "\n== Event Log ==\n";
    std::cout << "[mock] spectrum frame updated\n";
    std::cout << "[mock] conservative ISM card refreshed\n";
    std::cout << "[policy] no transmit or protected-content operation exposed\n";
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
  if (!has_arg(argc, argv, "--mock")) {
    std::cerr << "nevins-console currently runs with mock input only.\n";
    return 2;
  }

  if (has_arg(argc, argv, "--text")) {
    render_text_dashboard();
    return 0;
  }

  if (glfwInit() != GLFW_TRUE) {
    std::cerr << "GLFW initialization failed; falling back to terminal mock dashboard.\n";
    render_text_dashboard();
    return 0;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  GLFWwindow* window = glfwCreateWindow(
      1180,
      760,
      "Nevins Port - Device | Spectrum | Waterfall | Signal Cards | Capture Library | Policy | Event Log",
      nullptr,
      nullptr);

  if (window == nullptr) {
    glfwTerminate();
    std::cerr << "GLFW window creation failed; falling back to terminal mock dashboard.\n";
    render_text_dashboard();
    return 0;
  }

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  int frame = 0;
  const bool smoke = has_arg(argc, argv, "--smoke");
  while (glfwWindowShouldClose(window) == GLFW_FALSE) {
    int width = 0;
    int height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    render_dashboard(std::max(width, 1), std::max(height, 1), frame);
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
}
