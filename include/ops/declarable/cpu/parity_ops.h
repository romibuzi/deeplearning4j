//
// These ops are provided for features parity with TF
//
// @author raver119@gmail.com
//

#ifndef LIBND4J_PARITY_OPS_H
#define LIBND4J_PARITY_OPS_H

#include <memory>
#include <ops/ops.h>
#include <ops/declarable/declarable_ops.h>

namespace nd4j {
    namespace ops {


        DECLARE_OP(Concat, -1, 1){
            // do something here{
            Nd4jIndex _length;
            int _dimension = 0;

            // basic checks are happening here
            REQUIRE_OK(this->validateNonEmptyInput(block));

            // we want to ensure that all
            NDArray<T> *first = block.getVariables().at(0)->getNDArray();

            std::unique_ptr<int> shape(new int[first->_shapeInfo[0] * 2 + 4]);

            std::memcpy(shape.get(), first->_shapeInfo, (first->_shapeInfo[0] * 2 + 4) * sizeof(int));
            _length = shape::length(shape.get());

            std::unique_ptr<Nd4jPointer> buffers(new Nd4jPointer[block.getVariables().size()]);
            std::unique_ptr<Nd4jPointer> shapes(new Nd4jPointer[block.getVariables().size()]);

            buffers.get()[0] = (Nd4jPointer) first->_buffer;
            shapes.get()[0] = (Nd4jPointer) first->_shapeInfo;

            for (int e = 1; e < block.getVariables().size(); e++) {
                Variable<T> *var = block.getVariables().at(e);
                _length += var->getNDArray()->lengthOf();

                shape.get()[_dimension + 1] += var->getNDArray()->shapeOf()[_dimension];

                buffers.get()[e] = (Nd4jPointer) var->getNDArray()->_buffer;
                shapes.get()[e] = (Nd4jPointer) var->getNDArray()->_shapeInfo;
            }

            if (!block.getVariableSpace()->hasVariable(block.getNodeId()))
                throw "VariableSpace has no registered node";

            if (!this->allocateResult(block, shape.get())){
                nd4j_printf("Allocation failed: %i\n", block.getNodeId());
                throw "Allocation failed";
            }

            auto variable = block.getVariableSpace()->getVariable(block.getNodeId());

            concatCpuGeneric(_dimension, block.getVariables().size(), buffers.get(), shapes.get(), variable->getNDArray()->_buffer, variable->getNDArray()->_shapeInfo);

            return ND4J_STATUS_OK;
        }


        DECLARE_OP(BiasAdd, 2, 1) {
            REQUIRE_OK(this->validateNonEmptyInput(block));
            REQUIRE_OK(this->validateInput2D(block));

            NDArray<T> *x = block.getVariables().at(0)->getNDArray();
            NDArray<T> *y = block.getVariables().at(1)->getNDArray();

            if (x->isMatrix() && y->isVector()) {
                x->addiRowVector(y);
            } else if (y->isMatrix() && x->isVector()) {
                y->addiRowVector(x);
            }

            return ND4J_STATUS_OK;
        }


        DECLARE_OP(MatMul, 2, 1) {
            REQUIRE_OK(this->validateNonEmptyInput(block));

            // FIXME: we might want to have gemv/dot fallback here
            REQUIRE_OK(this->validateInput2D(block));


            NDArray<T> *x = block.getVariables().at(0)->getNDArray();
            NDArray<T> *y = block.getVariables().at(1)->getNDArray();

            if (x->isMatrix() && y->isMatrix()) {
                // gemm
            } else if (x->isMatrix() && y->isVector()) {
                // gemv
            } else if (x->isVector() && y->isMatrix()) {
                // gemv
            } else if (x->isVector() && y->isVector()) {
                // dot
            } else if (x->isVector() && y->isScalar()) {
                // elementwise mul
             } else if (x->isScalar() && y->isVector()) {
                // elementwise mul, reverse op
            }

            return ND4J_STATUS_OK;
        }


        DECLARE_OP(Conv2D, 2, 1) {
            // basically im2col + gemm
            return ND4J_STATUS_OK;
        }


        DECLARE_OP(Conv3D, 2, 1) {
            // cubic convo
            return ND4J_STATUS_OK;
        }



        DECLARE_OP(Relu, 1, 1) {
            REQUIRE_OK(this->validateNonEmptyInput(block));

            NDArray<T> *first = block.getVariables().at(0)->getNDArray();
            first->template applyTransform<simdOps::RELU<T>>();

            return ND4J_STATUS_OK;
        }

        DECLARE_OP(Reshape, 2, 1) {
            REQUIRE_OK(this->validateNonEmptyInput(block));

            return ND4J_STATUS_OK;
        }


        DECLARE_OP(Identity, 1, 1) {
            REQUIRE_OK(this->validateNonEmptyInput(block));

            NDArray<T> *first = block.getVariables().at(0)->getNDArray();
            first->template applyTransform<simdOps::Identity<T>>();

            return ND4J_STATUS_OK;
        }
		
		DECLARE_OP(Add, 2, 1) {
            REQUIRE_OK(this->validateNonEmptyInput(block));
            REQUIRE_OK(this->validateInputLengthMatch(block));

            NDArray<T> *x = block.getVariables().at(0)->getNDArray();
            NDArray<T> *y = block.getVariables().at(1)->getNDArray();			
	
			x->template applyPairwiseTransform<simdOps::Add<T>>(y, nullptr);		

            return ND4J_STATUS_OK;
        }
		
    }
}

#endif //LIBND4J_PARITY_OPS_H
