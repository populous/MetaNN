#include <MetaNN/meta_nn.h>
#include <data_gen.h>
#include <calculate_tags.h>
#include <cassert>
#include <iostream>
using namespace MetaNN;
using namespace std;

namespace
{
    using CommonInputMap = LayerInMap<LayerKV<LayerInput, Matrix<CheckElement, CheckDevice>>>;
    void test_sin_layer1()
    {
        cout << "Test sin layer case 1 ...\t";
        using RootLayer = MakeInferLayer<SinLayer>;
        static_assert(!RootLayer::IsFeedbackOutput);
        static_assert(!RootLayer::IsUpdate);

        RootLayer layer("root");
        Matrix<CheckElement, CheckDevice> in(2, 1);
        in.SetValue(0, 0, -0.27f);
        in.SetValue(1, 0, -0.41f);

        auto input = LayerInputCont<RootLayer>().Set<LayerInput>(in);

        LayerNeutralInvariant(layer);

        auto out = layer.FeedForward(input);
        auto res = Evaluate(out.Get<LayerOutput>());
        assert(fabs(res(0, 0) - std::sin(-0.27f)) < 0.001);
        assert(fabs(res(1, 0) - std::sin(-0.41f)) < 0.001);

        auto out_grad = layer.FeedBackward(LayerOutputCont<RootLayer>());
        static_assert(decltype(out_grad)::template IsValueEmpty<LayerInput>);

        LayerNeutralInvariant(layer);

        cout << "done" << endl;
    }

    void test_sin_layer2()
    {
        cout << "Test sin layer case 2 ...\t";
        using RootLayer = MakeTrainLayer<SinLayer, CommonInputMap, PFeedbackOutput>;
        static_assert(RootLayer::IsFeedbackOutput);
        static_assert(!RootLayer::IsUpdate);

        RootLayer layer("root");

        Matrix<CheckElement, CheckDevice> in(2, 1);
        in.SetValue(0, 0, -0.27f);
        in.SetValue(1, 0, -0.41f);

        auto input = LayerInputCont<RootLayer>().Set<LayerInput>(in);

        LayerNeutralInvariant(layer);

        auto out = layer.FeedForward(input);
        auto res = Evaluate(out.Get<LayerOutput>());
        assert(fabs(res(0, 0) - std::sin(-0.27f)) < 0.001);
        assert(fabs(res(1, 0) - std::sin(-0.41f)) < 0.001);

        Matrix<CheckElement, CheckDevice> grad(2, 1);
        grad.SetValue(0, 0, 0.1f);
        grad.SetValue(1, 0, 0.3f);
        auto out_grad = layer.FeedBackward(LayerOutputCont<RootLayer>().Set<LayerOutput>(grad));
        auto fb = Evaluate(out_grad.Get<LayerInput>());
        assert(fabs(fb(0, 0) - 0.1f * std::cos(-0.27f)) < 0.001);
        assert(fabs(fb(1, 0) - 0.3f * std::cos(-0.41f)) < 0.001);

        LayerNeutralInvariant(layer);
        cout << "done" << endl;
    }

    void test_sin_layer3()
    {
        cout << "Test sin layer case 3 ...\t";
        using RootLayer = MakeTrainLayer<SinLayer, CommonInputMap, PFeedbackOutput>;
        static_assert(RootLayer::IsFeedbackOutput);
        static_assert(!RootLayer::IsUpdate);

        RootLayer layer("root");

        vector<Matrix<CheckElement, CheckDevice>> op;

        LayerNeutralInvariant(layer);
        for (size_t loop_count = 1; loop_count < 10; ++loop_count)
        {
            auto in = GenTensor<CheckElement>(0.1f, 0.13f, loop_count, 3);

            op.push_back(in);

            auto input = LayerInputCont<RootLayer>().Set<LayerInput>(in);

            auto out = layer.FeedForward(input);
            auto res = Evaluate(out.Get<LayerOutput>());
            assert(res.Shape()[0] == loop_count);
            assert(res.Shape()[1] == 3);
            for (size_t i = 0; i < loop_count; ++i)
            {
                for (size_t j = 0; j < 3; ++j)
                {
                    assert(fabs(res(i, j) - std::sin(in(i, j))) < 0.0001);
                }
            }
        }

        for (size_t loop_count = 9; loop_count >= 1; --loop_count)
        {
            auto grad = GenTensor<CheckElement>(2, 1.1f, loop_count, 3);
            auto out_grad = layer.FeedBackward(LayerOutputCont<RootLayer>().Set<LayerOutput>(grad));

            auto fb = Evaluate(out_grad.Get<LayerInput>());

            auto in = op.back(); op.pop_back();
            for (size_t i = 0; i < loop_count; ++i)
            {
                for (size_t j = 0; j < 3; ++j)
                {
                    auto aim = grad(i, j) * std::cos(in(i, j));
                    assert(fabs(fb(i, j) - aim) < 0.00001f);
                }
            }
        }

        LayerNeutralInvariant(layer);
        cout << "done" << endl;
    }
    
    void test_sin_layer4()
    {
        cout << "Test sin layer case 4 (dummy grad input)...\t";
        using RootLayer = MakeTrainLayer<SinLayer, CommonInputMap, PFeedbackOutput>;
        static_assert(RootLayer::IsFeedbackOutput);
        static_assert(!RootLayer::IsUpdate);

        RootLayer layer("root");

        Matrix<CheckElement, CheckDevice> in(2, 1);
        in.SetValue(0, 0, -0.27f);
        in.SetValue(1, 0, -0.41f);

        auto input = LayerInputCont<RootLayer>().Set<LayerInput>(in);

        LayerNeutralInvariant(layer);

        auto out = layer.FeedForward(input);
        auto res = Evaluate(out.Get<LayerOutput>());
        assert(fabs(res(0, 0) - std::sin(-0.27f)) < 0.001);
        assert(fabs(res(1, 0) - std::sin(-0.41f)) < 0.001);

        auto out_grad = layer.FeedBackward(LayerOutputCont<RootLayer>());
        static_assert(decltype(out_grad)::template IsValueEmpty<LayerInput>);

        LayerNeutralInvariant(layer);
        cout << "done" << endl;
    }
}

namespace Test::Layer::Principal
{
    void test_sin_layer()
    {
        test_sin_layer1();
        test_sin_layer2();
        test_sin_layer3();
        test_sin_layer4();
    }
}