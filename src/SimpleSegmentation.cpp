#include <SimpleSegmentation.hpp>
#include <chrono>
#include <functional>
#include <opencv2/opencv.hpp>
#include <sstream>

using namespace std;

SimpleSegmentation::SimpleSegmentation(int argc, char *argv[])
    : _parser{{"simple-segmentation",
               "The default segmentation operation is to resize the input. "
               "Called without arguments is the same as: segmentation -e "
               "model.engine <input"}},
      _engineOption{
        {"-e", "--engine"},
        "Path for the engine to use. Default: ./model.engine",
        "engineFile",
        {"./mode.engine"},
      },
      _roiOption{
        {"--roi"},
        "Change segmentation operation to use the entire image scaled down and "
        "one "
        "region of interest at full input resolution.",
      },
      _overlayOption{
        {"-o", "--overlay"},
        "Overlay the resulting mask on top of the input image.",
      },
      _showOption{
        {"--show"},
        "Show original and segmented image.",
      },
      _saveCropsOption{
        {"--saveCrops"},
        "Save input images used.",
      },
      _saveMasksOption{
        {"--saveMasks"},
        "Save output masks.",
      },
      _simpleSaveOption{
        {"--simpleSave"},
        "Save only mask and image used side by side.",
      },
      _iouOption{{"--iou"}, "Set iou for NMS.", "value", {"0.7"}},
      _confOption{{"--conf"}, "Set minimum confidence.", "value", {"0.25"}},
      _useOption{{"--use"},
                 "Set indexes, separated by spaces between quotation "
                 "marks, to use for segmentation crop operation.",
                 "values",
                 {"17 18 22 23 24 25 26 28 29 30 31 32 33 34"}},
      _extensionOption{{"--extension"},
                       "The extension to use for saving outputs.",
                       "value",
                       {"jpg"}} {
  _parser.addHelpOption();
  _parser.addVersionOption();
  _parser.addPositionalArgument("INPUT",
                                "Path for the image to segment. Or path to a "
                                "directory containing images.");
  _parser.addOption(_engineOption);
  _parser.addOption(_roiOption);
  _parser.addOption(_overlayOption);
  _parser.addOption(_showOption);
  _parser.addOption(_saveCropsOption);
  _parser.addOption(_saveMasksOption);
  _parser.addOption(_simpleSaveOption);
  _parser.addOption(_iouOption);
  _parser.addOption(_confOption);
  _parser.addOption(_useOption);
  _parser.addOption(_extensionOption);
  _parser.parse(argc, argv);
}

int SimpleSegmentation::run() {
  const auto path = filesystem::absolute(_parser.positionalValues().front());

  const auto engine = filesystem::absolute(_parser.value(_engineOption));
  if (!filesystem::exists(engine)) {
    cerr << "Engine file: " << engine.string() << " not found." << endl;
    return 1;
  }
  cout << "Using engine: " << engine.string() << endl;
  cout << "Using path: " << path << endl;
  const auto images = getImages(path);
  if (images.empty()) {
    cerr << "No image found at " << path << endl;
    return 1;
  }
  const auto useRoi = _parser.isSet(_roiOption);
  const auto useIndex = _parser.isSet(_useOption);
  const auto resize = !useRoi && !useIndex;
  cout << "Performing segmentation "
       << (resize   ? "resizing input."
           : useRoi ? "using RoI."
                    : "slicing input.")
       << endl;
  const auto simpleSave = _parser.isSet(_simpleSaveOption);
  auto segmenter = Inference::Segmentation(engine, true, !simpleSave, 20);
  const auto simplePath = "runs/" + engine.stem().string();
  if (simpleSave) {
    filesystem::create_directories(simplePath);
  }
  const auto show = _parser.isSet(_showOption);
  Inference::Options options{
    .conf{_parser.value<float>(_confOption)},
    .iou{_parser.value<float>(_iouOption)},
    .overlay{_parser.isSet(_overlayOption)},
    .save{true},
    .saveCrops{_parser.isSet(_saveCropsOption)},
    .saveOutput{false},
    .saveMasks{_parser.isSet(_saveMasksOption)},
    .saveTimings{false},
  };

  auto extension = _parser.value(_extensionOption);
  if (!extension.starts_with('.')) {
    extension = "." + extension;
  }
  for (auto index = 1; const auto &imagePath : images) {
    cout << "Performing inference " << index << " of " << images.size() << endl;
    cout << "File: " << filesystem::absolute(imagePath) << endl;
    cv::Mat image = cv::imread(imagePath);
    if (show) {
      cv::namedWindow("Original Image", cv::WINDOW_NORMAL);
      cv::imshow("Original Image", image);
      cv::resizeWindow("Original Image", {800, 600});
      cv::moveWindow("Original Image", 50, 50);
    }

    cv::Mat segmented_image;
    Inference::Result<Inference::Segment> result;
    if (resize) {
      result = segmenter(image, segmented_image, options);
    } else if (useRoi) {
      result = segmenter(image, segmented_image, cv::Rect(), options);
    } else {
      // TODO: parse string array
      // result = segmenter(image, segmented_image, _args.indexes, options);
    }
    if (simpleSave) {
      const auto baseName = string(simplePath) + "/" +
                            string(filesystem::absolute(imagePath).stem()) +
                            "-mask";
      if (!options.overlay) {
        cv::Mat resized;
        cv::resize(image, resized, result.batches[0].resizedSize);
        cv::imwrite(baseName + extension, resized);
      }
      cv::imwrite(baseName + extension, segmented_image);
    }
    if (show) {
      cv::namedWindow("Segmented Image", cv::WINDOW_NORMAL);
      cv::imshow("Segmented Image", segmented_image);
      cv::resizeWindow("Segmented Image", {800, 600});
      cv::moveWindow("Segmented Image", 875, 50);
      cv::waitKey();
    }
    index++;
  }
  return 0;
}

bool SimpleSegmentation::isImage(const std::filesystem::directory_entry &file) {
  if (file.is_regular_file()) {
    const auto type = file.path().extension();
    return type.compare(".png") == 0 || type.compare(".jpg") == 0 ||
           type.compare(".jpeg") == 0;
  }
  return false;
}

std::vector<std::filesystem::path>
SimpleSegmentation::getImages(const std::filesystem::path &path) {
  std::vector<std::filesystem::path> images;
  if (!std::filesystem::exists(path)) {
    return images;
  }
  if (std::filesystem::is_directory(path)) {
    std::filesystem::directory_iterator it(path);
    for (const auto &content : it) {
      if (isImage(content)) images.push_back(content.path());
    }
  } else if (isImage(std::filesystem::directory_entry(path))) {
    images.push_back(path);
  }
  std::sort(images.begin(), images.end());
  return images;
}