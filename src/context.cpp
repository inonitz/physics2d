#include "context.hpp"




static globalContext ctx;


globalContext* getGlobalContext() { return &ctx; }