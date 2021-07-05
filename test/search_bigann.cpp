#include "diskann.h"


// search disk-based index 
// strategy is hnsw + ivf + pq + refine

/*
 * args:
 * 1. data type(string): float or uint8
 * 2. index path(string): a string end with '/' denotes the directory that where the index related file locates
 * 3. query data file(string): binary file of query data
 * 4. answer file(string): file name to store answer
 * 5. ground_truth file(string): file name where ground_truth stores
 * 6. nprobe(int): number of buckets to query
 * 7. refine nprobe(int): number of buckets to be candidate
 * 8. topk(int): number of answers 4 each query
 * 9. refine topk(int): number of answers 4 refine
 * 10. PQ.M(int): the number of codebook 4 each vector
 * 11. PQ.nbits(int): the number of sub-cluster 4 PQ codebook
 * 12. K1(int): number of centroids of the first round kmeans
 * 13. metric type(string): metric type
 */

int main(int argc, char** argv) {
    if (argc != 14) {
        std::cout << "Usage: << " << argv[0]
                  << " data_type(float or uint8)"
                  << " index path"
                  << " query data file"
                  << " answer file"
                  << " ground truth file"
                  << " nprobe"
                  << " refine nprobe"
                  << " topk"
                  << " refine topk"
                  << " PQ.M"
                  << " PQ.nbits"
                  << " K1"
                  << " metric type(L2 or IP)"
                  << std::endl;
        return 1;
    }
    // parse parameters
    std::string index_path(argv[2]);
    std::string query_file(argv[3]);
    std::string answer_file(argv[4]);
    std::string ground_truth_file(argv[5]);
    int nprobe = std::stoi(argv[6]);
    int refine_nprobe = std::stoi(argv[7]);
    int topk = std::stoi(argv[8]);
    int refine_topk = std::stoi(argv[9]);
    int PQM = std::stoi(argv[10]);
    int PQnbits = std::stoi(argv[11]);
    int K1 = std::stoi(argv[12]);
    auto metric_type = get_metric_type_by_name(std::string(argv[13]));
    assert(PQnbits == 8);

    if ('/' != *index_path.rbegin())
        index_path += '/';

    std::string hnsw_index_file = index_path + HNSW + INDEX + BIN;
    std::string pq_centroids_file = index_path + PQ_CENTROIDS + BIN;
    std::string bucket_centroids_file = index_path + BUCKET + CENTROIDS + BIN;

    std::vector<std::vector<uint8_t>> pq_codebook(K1);
    std::vector<std::vector<uint32_t>> meta(K1);
    load_pq_codebook(index_path, pq_codebook, K1);

    // for debug
    /*
    {
        int cid = -1, pos = -1;
        for (auto i = 0; i < K1 && cid < 0; i ++) {
            std::string ids_filei  = index_path + CLUSTER + std::to_string(i) + GLOBAL_IDS + BIN;
            std::ifstream ids_data_file_handlers = std::ifstream(ids_filei , std::ios::binary);
            uint32_t clu_id_size, clu_id_dim;
            ids_data_file_handlers.read((char*)&clu_id_size, sizeof(clu_id_size));
            ids_data_file_handlers.read((char*)&clu_id_dim, sizeof(clu_id_dim));
            uint32_t gid;
            for (auto j = 0; j < clu_id_size; j ++) {
                ids_data_file_handlers.read((char*)&gid, sizeof(uint32_t));
                if (0 == gid) {
                    cid = i;
                    pos = j;
                    break;
                }
            }
            ids_data_file_handlers.close();
        }
        std::cout << "find vector0 in cid: " << cid << " pos: " << pos << std::endl;
        std::cout << "pq.m = " << PQM << std::endl;
        std::cout << "show pq code of vector0:" << std::endl;
        for (auto si = 0; si < PQM; si ++) {
            std::cout << (int)pq_codebook[cid][pos * PQM + si] << " ";
        }
        std::cout << std::endl;
    }
    */




    load_meta(index_path, meta, K1);
    uint32_t bucket_num, dim;
    get_bin_metadata(bucket_centroids_file, bucket_num, dim);
    hnswlib::SpaceInterface<float>* space = nullptr;

    if (MetricType::L2 == metric_type) {
        space = new hnswlib::L2Space(dim);
    } else if (MetricType::IP == metric_type) {
        space = new hnswlib::InnerProductSpace(dim);
    }
    // load hnsw
    auto index_hnsw = std::make_shared<hnswlib::HierarchicalNSW<float>>(space, hnsw_index_file);

    if (argv[1] == std::string("float")) {
        PQ_Computer<float> pq_cmp; // pq computer
        Computer<float, float, float> dis_computer; // refine computer

        if (MetricType::L2 == metric_type) {
            pq_cmp = L2sqr<const float, const float, float>;
            dis_computer = L2sqr<const float, const float, float>;

            ProductQuantizer<CMax<float, uint64_t>, float, uint8_t> pq_quantizer(dim, PQM, PQnbits);
            pq_quantizer.load_centroids(pq_centroids_file);

            search_bigann<float, float, CMax<float, uint32_t>, CMax<float, uint64_t>>
                (index_path, query_file, answer_file, nprobe, refine_nprobe, topk, refine_topk, index_hnsw, pq_quantizer, K1, pq_cmp, pq_codebook, meta, dis_computer);
        } else if (MetricType::IP == metric_type) {
            pq_cmp = IP<const float, const float, float>;
            dis_computer = IP<const float, const float, float>;

            ProductQuantizer<CMin<float, uint64_t>, float, uint8_t> pq_quantizer(dim, PQM, PQnbits);
            pq_quantizer.load_centroids(pq_centroids_file);

            search_bigann<float, float, CMin<float, uint32_t>, CMin<float, uint64_t>>
                (index_path, query_file, answer_file, nprobe, refine_nprobe, topk, refine_topk, index_hnsw, pq_quantizer, K1, pq_cmp, pq_codebook, meta, dis_computer);
        }
        // calculate_recall<float>(ground_truth_file, answer_file, topk);
        recall<float, uint32_t>(ground_truth_file, answer_file, 100, topk);
    } else if (argv[1] == std::string("uint8")) {
        PQ_Computer<uint8_t> pq_cmp; // pq computer
        Computer<uint8_t, uint8_t, uint32_t> dis_computer; // refine computer

        if (MetricType::L2 == metric_type) {
            pq_cmp = L2sqr<const uint8_t, const float, float>;
            dis_computer = L2sqr<const uint8_t, const uint8_t, uint32_t>;

            ProductQuantizer<CMax<uint32_t, uint64_t>, uint8_t, uint8_t> pq_quantizer(dim, PQM, PQnbits);
            pq_quantizer.load_centroids(pq_centroids_file);

            search_bigann<uint8_t, uint32_t, CMax<uint32_t, uint32_t>, CMax<uint32_t, uint64_t>>
                (index_path, query_file, answer_file, nprobe, refine_nprobe, topk, refine_topk, index_hnsw, pq_quantizer, K1, pq_cmp, pq_codebook, meta, dis_computer);
        } else if (MetricType::IP == metric_type) {
            pq_cmp = IP<const uint8_t, const float, float>;
            dis_computer = IP<const uint8_t, const uint8_t, uint32_t>;

            ProductQuantizer<CMin<uint32_t, uint64_t>, uint8_t, uint8_t> pq_quantizer(dim, PQM, PQnbits);
            pq_quantizer.load_centroids(pq_centroids_file);

            search_bigann<uint8_t, uint32_t, CMin<uint32_t, uint32_t>, CMin<uint32_t, uint64_t>>
                (index_path, query_file, answer_file, nprobe, refine_nprobe, topk, refine_topk, index_hnsw, pq_quantizer, K1, pq_cmp, pq_codebook, meta, dis_computer);
        }
        // calculate_recall<uint32_t>(ground_truth_file, answer_file, topk);
        recall<uint32_t, uint32_t>(ground_truth_file, answer_file, 100, topk);
    }

    return 0;
}