## 版本信息

FastDDS：v3.1.1

FastCDR：v2.2.5

foonathan\_memory：v0.7-3

TinyXML2：v10.0.0

asio：默认

MinGW：v14.2.0，x86\_64-w64-mingw32（MSYS2中下载）

## 源码改动

### 1：编译选项关闭compile-tools

推荐cmakelist.txt中修改set(COMPILE\_TOOLS OFF)，关闭后不再编译fast-discovery-server，开了会符号报错，需要额外修改代码，非必用可以不开。

### 2：TimedMutes.hpp编译报错：undefined reference

MinGW在Windows上是非FastDDS官方支持的编译器，该cpp文件中进入错误的linux的define分支，使用linux相关库重新实现了相关符号，与MinGW冲突。

**解决方法：**

修改源码中include/utils/TimedMutes.hpp：&#x20;

新增自定义宏MINGW\_(cmakelist.txt中增加) 进入新分支，如下

```cpp
#if defined(_WIN32)

//#if defined(_MSC_FULL_VER) && _MSC_FULL_VER >= 193632528
#if defined(_MSC_FULL_VER) && _MSC_FULL_VER >= 193632528
#include <mutex>
#elif defined(MINGW_)
#include <mutex>
#else
#include <thread>
extern int clock_gettime(
        int,
        struct timespec* tv);
#endif // if defined(_MSC_FULL_VER) && _MSC_FULL_VER >= 193632528

......

#if defined(_WIN32)

#if defined(_MSC_FULL_VER) && _MSC_FULL_VER >= 193632528
using TimedMutex = std::timed_mutex;
using RecursiveTimedMutex = std::recursive_timed_mutex;
#elif defined(MINGW_)
using TimedMutex = std::timed_mutex;
using RecursiveTimedMutex = std::recursive_timed_mutex;
#else
......
#endif
```

### 3：依赖包符号导出/引入报错，“marked dllimport”、“marked dllexport”、“undefined reference”

官方源码中默认使用windows的符号导入导出规则，即使用\_\_declspec( dllexport )导出和\_\_declspec( dllimport )导入，而该符号体系在导出时可能会因为“-Werror”而报错，导入时可能不识别相关符号，所以需要做出对应修改。

解决方法（fastdds和fastcdr可用）：

根目录下cmakelist.txt中，在“Set warnings as errors”部分中设置编译选项时，在“-Werror”后方添加“-Wno-attributes”忽视来自符号导入导出的警告报错（此处可以最后再加，先把其余来自-Wno-attributes的错误能改则改，像符号导入导出一样不好改了再选择忽视）。

源码include/fastdds/fastdds\_dll.hpp中，修改导入导出宏：

```cpp
#if defined(_WIN32)
#if defined(EPROSIMA_ALL_DYN_LINK) || defined(FASTDDS_DYN_LINK)
#if defined(fastdds_EXPORTS)
#define FASTDDS_EXPORTED_API __declspec( dllexport )  //导出时使用windows导出语法__declspec( dllexport )，如使用__attribute__((visibility("default")))可能导致符号导出失败
#else
#define FASTDDS_EXPORTED_API  __attribute__((visibility("default"))) //导入时使用gcc语法
#endif // FASTDDS_SOURCE
#else
#define FASTDDS_EXPORTED_API
#endif // if defined(EPROSIMA_ALL_DYN_LINK) || defined(FASTDDS_DYN_LINK)
#else
#define FASTDDS_EXPORTED_API
#endif // _WIN32
```

### 4：DDSPublisherQos.cpp中重复定义与导出报错（可能出现）。

src/cpp/fastdds/publishers/qos/DDSPublisherQos.cpp中的 FASTDDS\_EXPORTED\_API const PublisherQos PUBLISHER\_QOS\_DEFAULT 可能报错'visibility' attribute ignored \[-Werror=attributes]重复导出。

解决方法： 删掉FASTDDS\_EXPORTED\_API符号导出修饰符，DDSPublisherQos.hpp中已有PUBLISHER\_QOS\_DEFAULT定义，此处重复定义并导出。

PS：const SubscriberQos SUBSCRIBER\_QOS\_DEFAULT同理。

### 5：asio编译链接错误

&#x20;GCC语言检查更严苛导致的，忽视即可，参照第三条，在 -Werror 后添加-Wno-cast-function-type -Wno-unused-variable -Wno-unused-parameter -Wno-unused-function。

### 6：RobustSharedLock.hpp中报错文件操作相关接口未定义

源码src/cpp/fastdds/utils/shared\_memory/RobustSharedLock.hpp和RobustExclusiveLock.hpp中有关文件操作部分只有msvc分支和其他（默认linux）分支，此处MinGW走linux分支故部分接口未定义。

解决方法：

引入宏定义MINGW\_控制分支，不走原本的windows和linux分支，同时复制一份msvc分支的代码到MinGW分支，并以此为基础修改代码。

```cpp
#ifdef  _MSC_VER
#include <io.h>
#elif defined(MINGW_)  //MinGW分支，也可以选择通过or选项并入到_MSC_VER分支
#include <io.h>
#else
#include <sys/file.h>
#endif // ifdef  _MSC_VER
......

#ifdef _MSC_VER   //MSVC分支
......
#elif defined(MINGW_) //MinGW分支
......
auto ret = _sopen_s(&test_exist, file_path.c_str(), _O_WRONLY, 0x0010, _S_IREAD | _S_IWRITE); //照抄MSVC部分源码，且该分支中所有_SH_DENYRW宏改为0x0010，因为该宏MinGW版io.h未定义
......
#else //Linux分支
......
#endif
```

### 7：SystemInfo.cpp中stat函数调用报错

src/cpp/fastdds/utils/SystemInfo.cpp中file\_exists()函数默认使用stat()判断文件存在，MinGW中stat.h中的stat结构体及相关代码中无对应函数可用，故报错，且fastdds其余代码中定义了stat宏，存在同名宏重定义问题。

解决方法：

使用宏定义增加windows分支（MinGW也走这一分支），不再默认调用stat.h判断文件存在，转而使用fileapi.h

```cpp
bool SystemInfo::file_exists(
        const std::string& filename)
{
#ifdef _WIN32
    // 在Windows平台下，使用GetFileAttributesA函数来检查文件是否存在
    DWORD fileAttributes = GetFileAttributesA(filename.c_str());
    if (fileAttributes == INVALID_FILE_ATTRIBUTES) {
        return false;
    }
    return !(fileAttributes & FILE_ATTRIBUTE_DIRECTORY);
#else
    struct stat s;
    return (stat(filename.c_str(), &s) == 0 && s.st_mode & S_IFREG);
#endif
}
```

### 8：i64未识别

src/cpp/fastdds/utils/TimedConditionVariable.cpp中宏定义中i64改为LL，因为GCC无法识别i64

### 9：FileWatch.cpp初始化缺失字段

thirdparty第3方库中FileWatch.hpp报错missing initializer for member '\_OVERLAPPED::InternalHigh' \[-Werror=missing-field-initializers]，未完整初始化，不影响使用，第三方代码不做修改，直接-Wno-missing-field-initializers忽视

### 10：严格符号优先级

&#x20;thirdparty第3方库FileWatch.hpp中407行||和&&判断中加()表明逻辑符号优先级，不然GCC报错（虽然有默认优先级顺序）

### 11\:tinyxml2报错（可能出现）

tinyxml2链接阶段报错，用msys2中的MinGW重新编译，并使用新的xml2依赖

### 12\:asio链接出错

连接阶段asio库报错未定义，需要正确链接到winsock2库，根目录cmakelist.txt增加target\_link\_libraries(fastdds PUBLIC ws2\_32 mswsock)，在480行附近。

### 13:重定义redefine问题

重定义问题，thirdparty第3方库中filewatch.hpp中stat宏改名为stat1，systeminfo.cpp中对stat宏的使用改为fileapi.h实现，不再使用自定义的stat，其余地方未使用该宏，可直接修改。

### 14：FastCDR中strncmp超限

cdr中字符串比较函数比较长度超限报错，不影响使用，直接-Wno-stringop-overread忽视

### 15\:codecvt库版本问题

src/cpp/fastdds/xtypes/serializers/json/dynamic\_data\_json.cpp中使用codecvt库进行宽字符转换，使用MSYS2/MinGW64编译报错。因为c11、17以及MSVC，MinGW之间关于codecvt库有兼容性问题，cmakelist.txt中修改c++标准无法解决该问题，引用该库会导致使用libfastdds.dll时无法定位程序输入点--codecvt\_utf8。

解决方法：

放弃使用codecvt库，使用stdlib库重新实现宽字符转换，重写所有用到codecvt库的地方。例如

```cpp
//#include <codecvt>  //注释掉这一行，不再引用codecvt库
......

std::wstring value;
ReturnCode_t ret = data->get_wstring_value(value, member_id); //需改动第780行与750行，此处以780行为例
if (RETCODE_OK == ret)
{
    // Insert UTF-8 converted value
    // std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;  //不再使用codecvt库进行宽字符转换
    // std::string utf8_value = converter.to_bytes(value);
    std::string utf8_value;
    int size_needed = std::wcstombs(nullptr, value.data(), 0);
    if (size_needed > 0) {
            utf8_value.resize(size_needed);
            std::wcstombs(&utf8_value[0], value.data(), size_needed); //使用stdlib进行宽字符转换
    }
    json_insert(member_name, utf8_value, output);
}
```

### 16：使用库的过程中出现vtable相关问题

移除Fast-DDS\include\fastdds\dds\topic\TypeSupport.hpp中所有的virtual关键字，使其不存在虚表。

而且常规使用中，注册类型都是直接使用的TypeSupport类，不需要再从TypeSupport派生类，完全可以去掉该类中所有的virtual声明。

如果需要从TypeSupport派生，就要另想办法。

## 编译过程

1：官网下载安装MSYS2

2：打开MSYS2命令行执行pacman -S mingw-w64-ucrt-x86\_64-gcc，下载MinGW，同时下载cmake、make等构建工具

3：打开MSYS2 UCRT64命令行工具，使用gcc -v查看MinGW版本

4：像常规linux系统编译操作一样，使用"cd"、“cmake ..”、"make"、"make install"等命令编译TinyXML2、FastCDR、foonathan\_memory、FastDDS即可

5：使用编译好的FastDDS和FastCDR库编译HelloWorldExample，编译通过并成功启动、发送消息、接收消息，则证明编译成功
