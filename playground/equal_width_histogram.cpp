// ----------------------------------------------------------------------------------------------------
#include <cassert>
#include <cstdint>
#include <algorithm>
#include <random>
// ----------------------------------------------------------------------------------------------------
#include <iostream>
#include <fstream>
//---------------------------------------------------------------------------
namespace {
//---------------------------------------------------------------------------
// TODO: only for text-to-image with float. :(
void generate_norm_histogram(const std::string& input_path, const std::string& output_path, const int num_bins) {
    // All datasets are in the common binary format that starts with
    // 8 bytes of data consisting of num_points(uint32_t) num_dimensions(uint32)
    // followed by num_pts X num_dimensions x sizeof(type) bytes of data stored one vector after another.
    std::ifstream input(input_path, std::ios::binary);
    uint32_t num_points;
    uint32_t num_dimensions;
    assert(input.is_open());
    input.read(reinterpret_cast<char*>(&num_points), sizeof(num_points));
    input.read(reinterpret_cast<char*>(&num_dimensions), sizeof(num_dimensions));
    assert(num_dimensions == 200 && "ONLY FOR TEST TO IMAGE.");
    std::cout << "number of points: " << num_points << std::endl;
    std::cout << "number of dimensions: " << num_dimensions << std::endl;
    std::vector<float> norm_vec(num_points, 0.0);

    std::cout << "Start to process the vector file." << std::endl;
    float ele;
    for (int i = 0; i < num_points; ++i) {
        float norm_squared = 0.0;
        for (int j = 0; j < num_dimensions; ++j) {
            input.read(reinterpret_cast<char*>(&ele), sizeof(ele));
            norm_squared += ele * ele;
        }
        norm_vec[i] = std::sqrt(norm_squared);
    }
    input.close();
    std::cout << "End of processing the vector file. Start to do in-memory sort." << std::endl;

    // ONLY IN-MEMORY SORTING!
    std::sort(norm_vec.begin(), norm_vec.end());
    const float min = norm_vec.front();
    const float max = norm_vec.back();
    assert(max <= 1.0);
    std::cout << "End of sorting. Start to build histogram." << std::endl;

    const int num_histogram_sperator = num_bins - 1;
    std::vector<uint64_t> range_counter(num_bins, 0);
    const float range_width = (max - min) / num_bins;
    // [range_start， range_end)， but for the last [range_start, max]
    for (const auto& norm : norm_vec) {
        const int index = (norm == max) ? num_histogram_sperator : (norm - min) / range_width;
        assert(index >= 0 && index <= num_histogram_sperator);
        ++range_counter[index];
    }

    std::vector<double> range_pecetage(num_bins, 0.0);
    for (int i = 0; i < range_pecetage.size(); ++i) range_pecetage[i] = 1.0 * range_counter[i] / num_points;
    std::cout << "End of building histogram." << std::endl;

    {
        // COUT output
        uint64_t sum_counter = 0;
        double sum_percentage = 0.0;
        for (int i = 0; i < range_pecetage.size(); ++i) {
            sum_counter += range_counter[i];
            sum_percentage += range_pecetage[i];
            std::cout << "[" << i << "]'s range ["
                      << min + range_width * i
                      << ", "
                      << ((i == range_pecetage.size() - 1) ? (max) : (min + range_width * (i + 1)))
                      << ((i == range_pecetage.size() - 1) ? "]" : ")")
                      << "  :=  "
                      << range_counter[i] << "  " << range_pecetage[i] * 100.0 << "%" << std::endl;
        }
        assert(sum_percentage == 1.0 || std::fabs(sum_percentage - 1.0) <= 0.00001);
        assert(sum_counter == num_points);
        std::cout << "The min of Norm: " << norm_vec.front() << std::endl;
        std::cout << "The max of Norm: " << norm_vec.back() << std::endl;
        std::cout << "The median of Norm: " << norm_vec[num_points / 2] << std::endl;
    }

    {
        // CSV output
        std::ofstream output(output_path, std::ios::binary);
        assert(output.is_open());
        output << "norm value range,counter,percentage" << std::endl;  // CSV's header
        for (int i = 0; i < range_pecetage.size(); ++i) {
            output << "["
                      << min + range_width * i
                      << " ~ "
                      << ((i == range_pecetage.size() - 1) ? (max) : (min + range_width * (i + 1)))
                      << ((i == range_pecetage.size() - 1) ? "]" : ")")
                      << "," << range_counter[i]
                      << "," << range_pecetage[i] << std::endl;
        }
        output.close();
    }
}
//---------------------------------------------------------------------------
} // namespace
//---------------------------------------------------------------------------
int main() {
    std::cout << "Please type in the input file input_path into the std::cin:" << std::endl;
    std::cout << "Example: \"/home/jigao/Desktop/Yandex.TexttoImage.base.10M.fdata\"" << std::endl;
    std::string input_path;
    std::cin >> input_path;
    std::cout << "The input input_path is: " << input_path << std::endl;

    std::cout << "Please type in the number of bins of the histogram:" << std::endl;
    std::cout << "Example: 20" << std::endl;
    int num_bins;
    std::cin >> num_bins;
    std::cout << "The input number of bins is: " << num_bins << std::endl;

    std::cout << "Please type in the output file input_path into the std::cin:" << std::endl;
    std::cout << "Example: \"/home/jigao/Desktop/histogram.csv\"" << std::endl;
    std::string output_path;
    std::cin >> output_path;
    std::cout << "The output_path is: " << output_path << std::endl;

    generate_norm_histogram(input_path, output_path, num_bins);

    return 0;
}
//---------------------------------------------------------------------------