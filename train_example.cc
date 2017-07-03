#include <tensorflow/c/c_api.h>
#include "tensorflow/cc/client/client_session.h"
#include "tensorflow/cc/ops/standard_ops.h"
#include "tensorflow/core/framework/tensor.h"

namespace My {
    // forward declaration
    template <typename T>
    class Matrix<T>;
}

namespace tf {
    using namespace ::tensorflow;
    using namespace ::tensorflow::ops;

    // Trying to mimic the naming convention of Keras
    tf::Output Dense(tf::ClientSession& session, const tf::Scope& scope, tf::Input inp, const int sizeOutput,
                     const char* opName) {
        const int num_columns = inp.tensor().shape().dim_size(1);
        auto tf_dt    = inp.data_type();
        auto weights  = tf::Variable(scope, {num_columns,sizeOutput}, tf_dt);
        auto assign_w = tf::Assign(scope, weights, tf::TruncatedNormal(scope, weights, tf_dt));
        auto biases   = tf::Variable(scope, {sizeOutput}, tf_dt);
        auto assign_b = tf::Assign(scope, biases, tf::ZerosLike(scope, biases));
        TF_CHECK_OK(session.Run({assign_w,assign_b}, nullptr));
        return tf::Add (opName && opName[0] ? scope.WithOpName(opName) : scope, tf::MatMul(scope, inp, weights), biases);
    }

    tf::Output Dense(tf::ClientSession& session, const tf::Scope& scope, const tf::Tensor& inp, const int sizeOutput,
                     const char* opName) {
        return Dense(session, scope, tf::Const(scope,inp), sizeOutput, opName);
    }

    void Assign (tf::Tensor& tensor, const ::My::Matrix<double>& mat) {
        auto tensor_data = tensor.flat<double>().data();
        const auto rows = mat.rows;
        const auto rowByteSize = mat.rowByteSize();
        for (int64_t r = 0; r<rows; r++) {
            memcpy(((char*)tensor_data) + r *rowByteSize, mat.rowData(r), rowByteSize);
        }
    }


}

int main(int argc, char* argv[]) {
    const auto nin     = 300; // number of inputs features
    const auto num_rows = 10000;
    const auto nhidden = 10; // number of hidden units
    const auto nout    = 10; // number of outputs
    const auto alpha   = 0.01; // learning rate
    My::Matrix<double> X(num_rows,nin);
    My::Matrix<double> Y_true(num_rows,nout);
    tf::Scope    scope = tf::Scope::NewRootScope();
    tf::DataType tf_dt = tf::DT_DOUBLE;
    tf::ClientSession session(scope);
    //auto inputLinesPlaceHolder  = tf::Placeholder(scope.WithOpName("input"),  tf_dt, tf::Placeholder::Shape({-1,nin}) /*shape = (None,nin)*/);
    //auto outputLinesPlaceHolder = tf::Placeholder(scope.WithOpName("output"), tf_dt, tf::Placeholder::Shape({-1,nout}) /*shape = (None,nout)*/);
    tf::Tensor tensor_X(tf_dt, {X.rows, X.cols});
    tf::Assign(tensor_X,X);
    tf::Tensor tensor_Y(tf_dt, {Y_true.rows, Y_true.cols});
    tf::Assign(tensor_Y,Y_true);
    auto tf_Y_true              = tf::Const(scope, tensor_Y);
    // Layer 1: Fully Connected. Input = nin. Output = nhidden.
    auto fc1             = tf::Dense (session, scope, tensor_X, nhidden, nullptr);
    // activation of layer 1
    auto tanh1           = tf::Tanh(scope.WithOpName("encoder"),fc1);

    // Layer 2: Fully Connected. Input = nhidden. Output = nout.
    auto fc2             = tf::Dense (session, scope, tanh1, nout, "decoder");

    auto cost_function   = tf::Mean(scope.WithOpName("cost"), tf::SquaredDifference(scope,fc2,tf_Y_true), -1/*reduce all dimensions*/);
    auto train_operation = tf::ApplyAdam(scope.WithOpName("train"),cost_function,
                                         0/*m*/,0/*v*/,0.9/*beta1_power*/,0.999/*beta2_power*/,alpha/*lr == learning rate*/,
                                         0.9/*beta1*/,0.999/*beta2*/,1e-6/*epsilon*/, 0.001 /*grad*/);
    for (int epoch=0; epoch < 40; epoch++) {
        TF_CHECK_OK(session.Run({train_operation}, nullptr));
    }
    std::vector<tf::Tensor> outputs;
    TF_CHECK_OK(session.Run({tanh1}, &outputs));
    return 0;

}
