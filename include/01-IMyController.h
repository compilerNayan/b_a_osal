#ifndef _IMYCONTROLLER_H_
#define _IMYCONTROLLER_H_

#include <StandardDefines.h>
#include "01-TestDto.h"
#include "03-RetDto.h"

DefineStandardPointers(IMyController)
class IMyController {
public:
    virtual ~IMyController() = default;

    virtual RetDto myFunee(TestDto x) = 0;

    virtual RetDto MyPostFunnee(TestDto x) = 0;
};

#endif