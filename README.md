<p align="left">
  <a href="https://github.com/ChunelFeng/CThreadPool"><img src="https://badgen.net/badge/langs/C++/cyan?list=1" alt="languages"></a>
  <a href="https://github.com/ChunelFeng/CThreadPool"><img src="https://badgen.net/badge/os/MacOS,Linux,Windows/cyan?list=1" alt="os"></a>
  <a href="https://github.com/ChunelFeng/CThreadPool/stargazers"><img src="https://badgen.net/github/stars/ChunelFeng/CThreadPool?color=cyan" alt="stars"></a>
  <a href="https://github.com/ChunelFeng/CThreadPool/network/members"><img src="https://badgen.net/github/forks/ChunelFeng/CThreadPool?color=cyan" alt="forks"></a>
</p>

<h1 align="center">
  CThreadPool 说明文档
</h1>

## 一. 简介
`CThreadPool` 是一个跨平台的、无任何三方依赖的、高性能的C++14（含以上版本）版本的线程池，也是 [CGraph](https://github.com/ChunelFeng/CGraph) 项目中使用的跨平台线程池组件功能的最小集。

经过CGraph和关联项目的长期迭代和验证，功能已经趋于稳定，且性能优异。因为咨询相关内容的朋友较多，故做为独立的仓库提供出来，方便大家使用。

由于是CGraph项目中的剥离出来的功能类，故在项目中保留了多处 `CGRAPH_*` 的命名方式，仅将 namespace 修改为 `CTP`，预计今后与CGraph相互独立更新。

本项目参考了[《C++并发编程实战（第二版）》](https://nj.gitbooks.io/c/content/) 中的部分内容，和github上的一些优秀实现。并在此基础上进行大量的改动、扩展和优化，在功能、性能和易用性上均有较大的提升。

在开发过程中，也沉淀了详细的说明文档（见下方 <b>推荐阅读</b>），以便于大家快速了解代码和思路，也请大家不吝指教。

## 二. 编译说明
* 本工程支持MacOS、Linux和Windows系统，无任何第三方依赖。推荐使用C++14（默认）或以上版本，不支持C++11或以下版本

* 使用`CLion`或使用`Visual Studio 15`(或以上版本)作为IDE的开发者，打开`CMakeLists.txt`文件作为工程，即可编译通过

* Linux环境开发者，在命令行模式下，输入以下指令，即可编译通过
```shell
$ git clone https://github.com/ChunelFeng/CThreadPool.git
$ cd CThreadPool
$ cmake . -Bbuild
$ cd build
$ make -j8
```

## 三. 使用Demo
```cpp
#include "src/CThreadPool.h"

using namespace CTP;

float add_by_5(float i) {
    return i + 5.0f;
}

void tutorial() {
    UThreadPool tp;
    int i = 6, j = 3;
    auto r1 = tp.commit([i, j] { return i - j; });
    std::future<float> r2 = tp.commit(std::bind(add_by_5, 8.5f));

    std::cout << r1.get() << std::endl;
    std::cout << r2.get() << std::endl;
}
```
更多使用方法，请参考 `tutorial.cpp` 中的例子和文档中的内容。

## 四. 推荐阅读
* [纯序员给你介绍图化框架的简单实现——线程池优化（一）](http://www.chunel.cn/archives/cgraph-threadpool-1-introduce)
* [纯序员给你介绍图化框架的简单实现——线程池优化（二）](http://www.chunel.cn/archives/cgraph-threadpool-2-introduce)
* [纯序员给你介绍图化框架的简单实现——线程池优化（三）](http://www.chunel.cn/archives/cgraph-threadpool-3-introduce)
* [纯序员给你介绍图化框架的简单实现——线程池优化（四）](http://www.chunel.cn/archives/cgraph-threadpool-4-introduce)
* [纯序员给你介绍图化框架的简单实现——线程池优化（五）](http://www.chunel.cn/archives/cgraph-threadpool-5-introduce)
* [纯序员给你介绍图化框架的简单实现——线程池优化（六）](http://www.chunel.cn/archives/cgraph-threadpool-6-introduce)

## 五. 关联项目
* [CGraph : A simple C++ DAG framework](https://github.com/ChunelFeng/CGraph)

------------
#### 附录-1. 版本信息
[2022.10.05 - v1.0.0 - Chunel]
* 提供线程池基本功能
* 提供对应的tutorial信息

[2022.10.07 - v1.0.1 - Chunel]
* 提供默认开启辅助线程的配置

------------
#### 附录-2. 联系方式
* 微信： ChunelFeng
* 邮箱： chunel@foxmail.com
* 源码： https://github.com/ChunelFeng/CThreadPool
* 论坛： www.chunel.cn

![CGraph Author](https://github.com/ChunelFeng/CThreadPool/blob/main/doc/image/CThreadPool%20Author.jpg)
