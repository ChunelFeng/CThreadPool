/***************************
@Author: Chunel
@Contact: chunel@foxmail.com
@File: MyFunction.h
@Time: 2021/9/2 11:20 下午
@Desc:
***************************/

#ifndef CGRAPH_MYFUNCTION_H
#define CGRAPH_MYFUNCTION_H


int add(int i, int j) {
    return i + j;
}

static float minusBy5(float i) {
    return i - 5.0f;
}


class MyFunction {
public:
    std::string concat(std::string& str) const {
        return info_ + str;
    }

    static int multiply(int i, int j) {
        return i * j;
    }

private:
    std::string info_ = "MyFunction : ";
};

#endif //CGRAPH_MYFUNCTION_H
