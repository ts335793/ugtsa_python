#pragma once

#include "ugtsa/computation_graphs/computation_graph/computation_graph.h"

namespace computation_graphs {
namespace basic_computation_graph {

class ComputationGraph : public computation_graphs::computation_graph::ComputationGraph {
    struct Transformation {
        std::string seed;
        int seed_size;
        std::vector<std::string> inputs;
        std::vector<std::vector<int>> input_shapes;
        std::vector<int> input_sizes;
        std::vector<tensorflow::DataType> input_types;
        std::vector<std::string> input_gradients;
        std::string output;
        std::vector<int> output_shape;
        int output_size;
        tensorflow::DataType output_type;
        std::string output_gradient;
        std::string update_model_gradient_accumulators;
    };

    struct Node {
        int transformation;
        std::vector<std::vector<int>> inputs;
        std::vector<int> output;
    };

    struct Batch {
        int nodes_end;
        std::vector<std::vector<long long>> seeds;
    };

    template <typename T>
    void copy_input_into_tensor(int row, int row_size, std::vector<int> &node_indices, tensorflow::Tensor &tensor) {
        auto view = tensor.flat<T>();

        int view_iterator = row * row_size;

        for (int node_index : node_indices) {
            for (int x : nodes[node_index].output) {
                view(view_iterator++) = *((T *)&x);
            }
        }

        while(view_iterator < (row + 1) * row_size) view(view_iterator++) = 0.;
    }

    template <typename T>
    void copy_vector_into_tensor(int row, int row_size, std::vector<int> &buffer, tensorflow::Tensor &tensor) {
        auto view = tensor.flat<T>();

        int view_iterator = row * row_size;

        for (int x : buffer) {
            view(view_iterator++) = *((T *)&x);
        }
    }

    tensorflow::Session* session;
    std::string training_name;
    tensorflow::Tensor training_tensor;

    std::default_random_engine generator;
    std::vector<Transformation> transformations;
    std::vector<Batch> batches;

public:
    std::vector<Node> nodes;

    ComputationGraph(tensorflow::Session* session, std::string training_name, bool training);

    int transformation(
        std::string seed,
        int seed_size,
        std::vector<std::string> inputs,
        std::vector<std::vector<int>> input_shapes,
        std::vector<tensorflow::DataType> input_types,
        std::vector<std::string> input_gradients,
        std::string output,
        std::vector<int> output_shape,
        tensorflow::DataType output_type,
        std::string output_gradient,
        std::string update_model_gradient_accumulators);
    int matrix(Eigen::VectorXi vector);
    int matrix(Eigen::VectorXf vector);
    int matrix(Eigen::MatrixXi matrix);
    int matrix(Eigen::MatrixXf matrix);
    int transformation_run(int transformation, std::vector<std::vector<int>> inputs);
    void accumulate_model_gradients(int first_node, std::vector<int> y_grad_indices, std::vector<Eigen::VectorXf, Eigen::aligned_allocator<Eigen::VectorXf>> y_grad_values);
    void run_batch();
    Eigen::VectorXf value(int index);
};

}
}