#ifndef VIRTUALSCANNER_H
#define VIRTUALSCANNER_H

#include <cstddef>

/**
 * @brief 虚拟扫描仪类，用于测试和开发
 * 
 * 该类模拟真实扫描仪的行为，允许加载测试数据并模拟设备响应
 */
class VirtualScanner
{
public:
    /**
     * @brief 构造函数，自动加载测试数据
     */
    VirtualScanner();
    
    /**
     * @brief 析构函数，释放所有资源
     */
    ~VirtualScanner();
    
    /**
     * @brief 打开虚拟扫描仪连接
     * @return 0表示成功，其他值表示错误码
     */
    int open();
    
    /**
     * @brief 关闭虚拟扫描仪连接
     * @return 0表示成功，其他值表示错误码
     */
    int close();
    
    /**
     * @brief 向虚拟扫描仪写入数据
     * @param buf 数据缓冲区
     * @param len 数据长度
     * @return 成功写入的字节数
     */
    int write(const unsigned char* buf, size_t len);
    
    /**
     * @brief 从虚拟扫描仪读取数据
     * @param buf 接收数据的缓冲区，传入nullptr仅返回可用数据大小
     * @param len 缓冲区大小
     * @return 成功读取的字节数或可用数据大小
     */
    int read(unsigned char* buf, size_t len);
    
    /**
     * @brief 向虚拟扫描仪发送控制命令
     * @param buf 控制命令缓冲区
     * @param len 命令长度
     * @return 操作结果代码
     */
    int ioctl(unsigned char* buf, size_t len);

    /**
     * @brief 设置虚拟扫描仪的测试结果数据
     * @param buf 数据缓冲区，所有权转移至本类
     * @param len 数据长度
     */
    void setResult(unsigned char* buf, size_t len);

private:
    /**
     * @brief 加载测试数据
     */
    void loadTestData();
    
    /**
     * @brief 清理结果数据，释放内存
     */
    void clearResult();
    
    unsigned char* _result;
    size_t _size;
};

#endif // VIRTUALSCANNER_H
