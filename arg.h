#ifndef ARG_H
#define ARG_H
#include <iostream>
#include <stdio.h>
#include <cstdlib>
#include <iomanip>

#include <string>
#include <sstream>
using namespace std;
/**
 * @class Arg
 * Этот класс предназначен для ввода числовых параметров программы различных типов
 */
template <typename T>
class Arg
{
    T val; ///< Значение
    T *valDef; ///< Значение по-умолчанию
    string *opt;///< Как должен выглядеть аргумент при запуске программы
    string *format;///< Спецификатор для ввода параметров (см. документацию [scanf](http://www.cplusplus.com/reference/cstdio/scanf/))
    T *gt;///< нижняя граница для параметра (Greater Than)
    T *lt;///< верхняя граница для параметра (Less Than)
public:
    /**
     * @brief Конструктор
     * @param[in] defVal Значение параметра по-умолчанию
     * @param[in] opt_ Как должен выглядеть аргумент при запуске программы
     * @param[in] format_ Спецификатор (см. документацию [scanf](http://www.cplusplus.com/reference/cstdio/scanf/))
     * @param[in] greater Нижняя граница для числа. По-умолчанию границы нет.
     * @param[in] less Верхняя граница для числа. По-умолчанию границы нет.
     */
    Arg(const T defVal, const string opt_, const string format_,  const T* greater=NULL,const T* less=NULL)
        : val(defVal) {
        opt = new string(opt_);
        format = new string(format_);
        valDef = new T(defVal);
        if(greater) gt = new T(*greater); else gt=NULL;
        if(less)    lt = new T(*less);    else lt=NULL;
        cout << "\t" << *opt << "[" << val << "]" << endl;
    }
    /**
     * @brief Ввод параметра
     * Определяет, является ли текущая строка нужным идентефикатором
     * @param[in] argv один аргумент программы
     * @return true, если была распознана строка-идентификатор и знчение удалось прочитать
     */
    bool input(const char* argv){
        if(exists(argv))
        {
            if(sscanf( argv + opt->length(), format->c_str(), &val)){
                if((gt!=NULL && *gt>val))
                    val=*valDef;
                else if((lt!=NULL && *lt<val))
                    val=*valDef;
                cout << " and " << val << " assigned."<<endl;
                delete format, opt;
                delete valDef;
                return true;
            }
        }
        return false;
    }
    /**
     * @brief operator T
     * Маскировка данного класса под используемый тип
     */
    operator T(){
        return val;
    }
    /**
     * @brief operator =
     * Маскировка данного класса под используемый тип
     * @param[in] newVal Новое значение параметра
     * @return Новое значение параметра
     */
    T operator =(T newVal){
        val=newVal;
        return val;
    }
    /**
     * @brief Поиск аргумента
     * Ищет аргумент в заданной строке
     * @param[in] argv Один аргумент программы
     * @return \r true, если аргумент найден
     */
    bool exists(const char* argv){
        return (opt->compare(0,opt->length(),argv,opt->length())==0);
    }
    ~Arg(){
        delete gt,lt;
    }
};

#endif // ARG_H
