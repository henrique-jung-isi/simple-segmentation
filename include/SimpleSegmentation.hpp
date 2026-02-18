#pragma once

#include <arg_parser/ArgParser.hpp>
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
  std::vector<int> indexes{
    17, 18, 22, 23, 24, 25, 26, 28, 29, 30, 31, 32, 33, 34};
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
  ArgParser _parser;
  ArgOption _engineOption;
  ArgOption _roiOption;
  ArgOption _overlayOption;
  ArgOption _showOption;
  ArgOption _saveCropsOption;
  ArgOption _saveMasksOption;
  ArgOption _simpleSaveOption;
  ArgOption _iouOption;
  ArgOption _confOption;
  ArgOption _useOption;
  ArgOption _extensionOption;

public:
  SimpleSegmentation(int argc, char *argv[]);

  int run();
  bool isImage(const std::filesystem::directory_entry &content);
  std::vector<std::filesystem::path>
  getImages(const std::filesystem::path &path);
};
