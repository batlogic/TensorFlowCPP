#include <tensorflow/c/c_api.h>
#include "tensorflow/cc/client/client_session.h"
#include "tensorflow/cc/ops/standard_ops.h"
#include "tensorflow/core/framework/tensor.h"

namespace tf {
    using namespace ::tensorflow;
    using namespace ::tensorflow::ops;

    tf::Output FullyConnectedLinear(tf::ClientSession& session, const tf::Scope& scope, tf::Input inp, const int sizeOutput) {
        const int size_input = inp.tensor().shape().dim_size(1); // dim_size(1) == number of columns
        auto tf_dt    = inp.data_type();
        auto weights  = tf::Variable(scope, {size_input,sizeOutput}, tf_dt);
        auto assign_w = tf::Assign(scope, weights, tf::TruncatedNormal(scope, weights, tf_dt));
        auto biases   = tf::Variable(scope, {sizeOutput}, tf_dt);
        auto assign_b = tf::Assign(scope, biases, tf::ZerosLike(scope, biases));
        TF_CHECK_OK(session.Run({assign_w,assign_b}, nullptr));
        return tf::Add (scope, tf::MatMul(scope, inp, weights), biases);
    }
}

int main(int argc, char* argv[]) {
    const auto nin     = 300; // number of inputs features
    const auto nhidden = 10; // number of hidden units
    const auto nout    = 10; // number of outputs
    const auto alpha   = 0.01; // learning rate
    tf::Scope    scope = tf::Scope::NewRootScope();
    tf::DataType tf_dt = tf::DT_DOUBLE;
    tf::ClientSession session(scope);
    auto inputLinesPlaceHolder  = tf::Placeholder(scope.WithOpName("input"),  tf_dt, tf::Placeholder::Shape({-1,nin}) /*shape = (None,nin)*/);
    auto outputLinesPlaceHolder = tf::Placeholder(scope.WithOpName("output"), tf_dt, tf::Placeholder::Shape({-1,nout}) /*shape = (None,nout)*/);
    // Layer 1: Fully Connected. Input = nin. Output = nhidden.
    auto fc1 = tf::FullyConnectedLinear(session,scope,inputLinesPlaceHolder,nhidden);

    // activation of layer 1
    auto tanh1   = tf::Tanh(scope,fc1);

    // Layer 2: Fully Connected. Input = nhidden. Output = nout.
    auto fc2 = tf::FullyConnectedLinear(session,scope,tanh1,nout);
    // what should be the axis?
    auto cost_function = tf::Mean(scope,tf::SquaredDifference(scope,fc2,outputLinesPlaceHolder),1/*axis*/);
    auto train_operation = tf::ApplyAdam(scope.WithOpName("train"),cost_function,
                                         0/*m*/,0/*v*/,0.9/*beta1_power*/,0.999/*beta2_power*/,alpha/*lr == learning rate*/,
                                         0.9/*beta1*/,0.999/*beta2*/,1e-6/*epsilon*/, 0.001 /*grad*/);

    //TF_CHECK_OK(session.Run({{"input", x}, {"output", y}}, {"train"}, nullptr));
    return 0;

}
