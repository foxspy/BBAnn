#include "lib/bbannlib2.h"

int main(int argc, char **argv) {
/*  std::cout << " raw_data_bin_file: " << dataFilePath_
            << " output_path: " << indexPrefix_ << " hnsw.M: " << para.hnswM
            << " hnsw.efConstruction: " << para.hnswefC << " K1: " << para.K1
            << std::endl;*/
  bbann::BBAnnParameters para;
  para.dataFilePath = argv[1];
  para.indexPrefixPath =  para.dataFilePath + "index_";
  para.hnswM = 16;
  para.hnswefC = 200;
  para.efSearch = 200;
  auto index =
      std::make_unique<bbann::BBAnnIndex2<float, float>>(MetricType::L2);
  bbann::BBAnnIndex2<float, float>::BuildIndex(para);
  return 0;
}