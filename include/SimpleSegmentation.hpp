#pragma once

#include <filesystem>
#include <isisim_inference/segmentation>
#include <string>
#include <vector>

struct Args {
  std::string enginePath{std::filesystem::absolute("model.engine")};
  std::string path{"."};
  bool resize{true};
  bool useRoi{false};
  bool showHelp{false};
  bool show{false};
  bool simpleSave{false};
  std::string extension{".jpg"};
  std::vector<int> indexes{17, 18, 22, 23, 24, 25, 26,
                           28, 29, 30, 31, 32, 33, 34};
  Inference::Options options{
      .conf{0.25},
      .iou{0.7},
      .overlay{false},
      .save{true},
      .saveCrops{false},
      .saveOutput{false},
      .saveMasks{false},
      .saveTimings{false},
  };
  std::vector<std::string> error{};
};

class SimpleSegmentation {
  Args _args;

public:
  SimpleSegmentation(int argc, char *argv[]);

  int run();
  bool isImage(const std::filesystem::directory_entry &content);
  std::vector<std::filesystem::path>
  getImages(const std::filesystem::path &path);
  Args parseArgs(int argc, char *argv[]);
  void showHelpMessage();
};
