#include <simple_segmentation/SimpleSegmentation.hpp>

int main(int argc, char *argv[]) {
  SimpleSegmentation segmentation(argc, argv);
  return segmentation.run();
}