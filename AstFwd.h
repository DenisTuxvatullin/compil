#pragma once
#include <string>
#include "DataType.h"

class IAst;
class AstError;
class AstVariable;
class AstUnaryExpression;
class AstBinaryExpression;
class AstAssignmentExpression;
class AstList;
class AstFunction;
class AstReturn;
class AstGlobal;
class AstFunctionCall;
class AstIf;
class AstWhile;
class AstFor;
class AstNewArray;
class AstArrayValue;
class AstArrayAssign;
class AstNewDict;
class AstDictValue;
class AstDictAssign;
class AstConstantDecl;
class AstFor;

template<class T, ATOMIC_TYPE DATA_TYPE> class AstConstant;
typedef AstConstant<double, ATOMIC_TYPE::TYPE_FLOAT> AstFloatConst;
typedef AstConstant<std::string, ATOMIC_TYPE::TYPE_STRING> AstStringConst;
typedef AstConstant<int, ATOMIC_TYPE::TYPE_INT> AstIntConst;
typedef AstConstant<bool, ATOMIC_TYPE::TYPE_BOOL> AstBoolConst;
