#include <SimpleSegmentation.hpp>
#include <chrono>
#include <functional>
#include <opencv2/opencv.hpp>
#include <sstream>

using namespace std;

SimpleSegmentation::SimpleSegmentation(int argc, char *argv[])
    : _args{parseArgs(argc, argv)} {}

int SimpleSegmentation::run() {
  if (!_args.error.empty()) {
    cerr << "Unkown option: " << _args.error[0] << endl;
    showHelpMessage();
    return 1;
  }
  if (_args.path.empty()) {
    cerr << "Input path not given." << endl;
    showHelpMessage();
    return 1;
  }
  if (_args.showHelp) {
    showHelpMessage();
    return 0;
  }

  const auto images = getImages(_args.path);

  cout << "Using engine: " << _args.enginePath << endl;
  if (!filesystem::exists(_args.enginePath)) {
    cerr << "Engine file: " << _args.enginePath << " file not found." << endl;
    return 1;
  }
  cout << "Using path: " << _args.path << endl;
  if (images.empty()) {
    cerr << "No image found at " << _args.path << endl;
    return 1;
  }
  cout << "Performing segmentation "
       << (_args.useRoi   ? "using RoI."
           : _args.resize ? "resizing input."
                          : "slicing input.")
       << endl;

  auto index = 1;
  auto segmenter =
      Inference::Segmentation(_args.enginePath, true, !_args.simpleSave, 20);
  const auto simplePath =
      "runs/" + string(filesystem::path(_args.enginePath).stem());
  if (_args.simpleSave) {
    filesystem::create_directories(string(simplePath));
  }

  for (const auto &imagePath : images) {
    cout << "Performing inference " << index << " of " << images.size() << endl;
    cout << "File: " << filesystem::absolute(imagePath) << endl;
    cv::Mat image = cv::imread(imagePath);
    if (_args.show) {
      cv::namedWindow("Original Image", cv::WINDOW_NORMAL);
      cv::imshow("Original Image", image);
      cv::resizeWindow("Original Image", {800, 600});
      cv::moveWindow("Original Image", 50, 50);
    }

    cv::Mat segmented_image;
    Inference::Result<Inference::Segment> result;
    if (_args.useRoi) {
      result = segmenter(image, segmented_image, cv::Rect(), _args.options);
    } else if (_args.resize) {
      result = segmenter(image, segmented_image, _args.options);
    } else {
      result = segmenter(image, segmented_image, _args.indexes, _args.options);
    }
    if (_args.simpleSave) {
      const auto baseName = string(simplePath) + "/" +
                            string(filesystem::absolute(imagePath).stem()) +
                            "-mask";
      // if (!options.overlay) {
      //   cv::Mat resized;
      //   cv::resize(image, resized, result.batches[0].resizedSize);
      //   cv::imwrite(baseName + extension, resized);
      // }
      cv::imwrite(baseName + _args.extension, segmented_image);
    }
    if (_args.show) {
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
      if (isImage(content))
        images.push_back(content.path());
    }
  } else if (isImage(std::filesystem::directory_entry(path))) {
    images.push_back(path);
  }
  return images;
}

void SimpleSegmentation::showHelpMessage() {
  cout
      << "Usage:\n"
         "  segmentation [OPTIONS] [INPUT]\n\n"

         "  The default segmentation operation is to resize the input.\n"
         "  Called without arguments is the same as:\n"
         "  segmentation -e model.engine <input>\n\n"

         "Input:\n"
         " Path for the image to segment. Or path to a directory containing "
         "images.\n\n"

         "Options:\n"
         "  -h, --help         Show help.\n"
         "  -c, --crop         Change segmentation operation to crop.\n"
         "  --roi              Change segmentation operation to use the\n"
         "                     entire image scaled down and one region of\n"
         "                     interest at full input resolution.\n\n"

         "  -e <file>          Path for the engine to use.\n"
         "                     Default: ./model.engine\n\n"

         "  -o                 Overlay the resulting mask on top of the input\n"
         "                     image.\n\n"

         "  --show             Show original and segmented image.\n"
         "  --saveCrops        Save input images used.\n"
         "  --saveMasks        Save output masks.\n"
         "  -s                 Save everything.\n"
         "  --simpleSave       Save only mask and image used side by side.\n"
         "  --iou <value>      Set iou for NMS.\n"
         "                     Default: 0.7\n\n"

         "  --conf <value>     Set minimum confidence.\n"
         "                     Default: 0.25\n\n"

         "  --use <values>     Set indexes, separated by spaces between\n"
         "                     quotation marks, to use for segmentation crop\n"
         "                     operation.\n"
         "                     Default: \"17 18 22 23 24 25 26 28 29 30 31 32\n"
         "                               33 34\"\n";
}

Args SimpleSegmentation::parseArgs(int argc, char *argv[]) {
  Args args;
  for (int i = 0; i < argc; i++) {
    const auto arg = string(argv[i]);
    if (arg == "-h" || arg == "--help") {
      return Args{.showHelp{true}};
    } else if (arg == "-e" && i + 1 < argc) {
      args.enginePath = filesystem::absolute(argv[i + 1]);
    } else if (arg == "-o") {
      args.options.overlay = true;
    } else if (arg == "--saveCrops") {
      args.options.saveCrops = true;
    } else if (arg == "--saveOutput") {
      args.options.saveOutput = true;
    } else if (arg == "--saveMasks") {
      args.options.saveMasks = true;
    } else if (arg == "-s") {
      args.options.saveCrops = true;
      args.options.saveOutput = true;
      args.options.saveMasks = true;
    } else if (arg == "--iou" && i + 1 < argc) {
      args.options.iou = stof(argv[i + 1]);
    } else if (arg == "--conf" && i + 1 < argc) {
      args.options.conf = stof(argv[i + 1]);
    } else if (arg == "--use" && i + 1 < argc) {
      args.indexes.clear();
      stringstream ss(argv[i + 1]);
      string index;
      while (getline(ss, index, ' ')) {
        args.indexes.push_back(stoi(index));
      }
    } else if (arg == "-c" || arg == "--crop") {
      args.resize = false;
      args.useRoi = false;
    } else if (arg == "--roi") {
      args.useRoi = true;
      args.resize = false;
    } else if (arg == "--show") {
      args.show = true;
    } else if (arg == "--simpleSave") {
      args.simpleSave = true;
    } else if (arg == "--png") {
      args.extension = ".png";
    } else if (arg.starts_with('-')) {
      args.error.push_back(arg);
    } else {
      args.path = arg;
    }
  }
  return args;
}
