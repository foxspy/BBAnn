#pragma once

// type definations
//

enum class MetricType {
  None = 0,
  L2 = 1,
  IP = 2,
};

enum class DataType {
  None = 0,
  INT8 = 1,
  FLOAT = 2,
};

enum class QuantizerType {
  None = 0,
  PQ = 1,
  PQRES = 2,
};

enum class LevelType {
  FIRST_LEVEL = 0,
  SECOND_LEVEL = 1,
  THIRTH_LEVEL = 2,
  BALANCE_LEVEL = 3,
  FINAL_LEVEL = 4,
};

namespace bbann {
struct BBAnnParameters {
  std::string dataFilePath;
  std::string indexPrefixPath;
  std::string queryPath;
  std::string groundTruthFilePath;
  MetricType metric = MetricType::L2;
  int K = 20; // top k.
  int hnswM = 16;
  int hnswefC = 200;
  int K1 = 20;
  int blockSize = 4096;
  int nProbe = 2;
  int efSearch = 200;
  int rangeSearchProbeCount = 20;
  int aio_EventsPerBatch = 512;
  int sample = 1;
  bool vector_use_sq = false;
  double radiusFactor = 1.0;
  bool use_hnsw_sq = false;
};

} // namespace bbann
