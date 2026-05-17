#ifndef MY_CONTROLLER
#define MY_CONTROLLER

#include "01-IMyController.h"

/* @RestController */
/* @RequestMapping("/myUrlTee") */
class MyController final : public IMyController {
  public:

  /* @GetMapping("/mysomeget2ee") */
  RetDto myFunee(/* @RequestBody */ TestDto x) override{
    std_println(x.a.value());
    std_println(x.b.value());
    std_println(x.c.value().c_str());
    return RetDto();
  }

  /* @PostMapping("/somePost2ee") */
  RetDto MyPostFunnee(/* @RequestBody */ TestDto x) override{
    std_println("MyPostFunnee called");
    std_println(x.a.value());
    std_println(x.b.value());
    std_println(x.c.value().c_str());
    RetDto retDto;
    retDto.a = 1;
    retDto.b = 2;
    retDto.c = 5;
    return retDto;
  }
};

#endif