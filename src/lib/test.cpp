#include "lib/bbannlib2.h"



template<typename T>
void
GenWithInput(std::string& input, int& dim, int& size, std::vector<T>& data) {
  std::ifstream fs(input.c_str(), std::ios::binary);
  fs.read((char*)&dim, sizeof(uint32_t));
  fs.seekg(0, std::ios::end);
  size = fs.tellg() / (dim + 1) / sizeof(float);
  data.resize(size * dim);
  fs.seekg(0, std::ios::beg);
  for (int i = 0; i < size; i++) {
    fs.seekg(4, std::ios::cur);
    fs.read((char *)(data.data() + i*dim), dim*sizeof(T));
  }
  fs.close();
}


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
  auto index = std::make_unique<bbann::BBAnnIndex2<float, float>>(MetricType::L2);
  bbann::BBAnnIndex2<float, float>::BuildIndex(para);

  int dim, n_query;
  std::vector<float> queryData;
  std::string query_path = para.dataFilePath + "_query.fvecs";
  GenWithInput(query_path, dim, n_query, queryData);

  int top_k, n_result;
  std::vector<int> ground;
  std::string ground_path = para.dataFilePath + "_groundtruth.ivecs";
  GenWithInput(para.dataFilePath, top_k, n_result, ground);

  uint32_t* answer_ids = new uint32_t[top_k * n_query];
  float* dist = new float [top_k * n_query];
  bbann::BBAnnIndex2<float, float> bbann(MetricType::L2);
  bbann.BatchSearchCpp(queryData.data(), dim, n_query, top_k, para, answer_ids, dist);

  return 0;
}