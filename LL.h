#pragma once
#include <memory>
#include "Ast.h"
#include "TokenStream.h"

std::shared_ptr<IAst> LLParseNew(TokenStream &tk);
