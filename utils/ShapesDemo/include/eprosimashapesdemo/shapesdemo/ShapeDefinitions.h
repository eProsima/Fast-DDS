#ifndef SHAPEDEFINITIONS_H
#define SHAPEDEFINITIONS_H

/**
 * @brief The SD_COLOR enum, the different colors.
 */
enum SD_COLOR
{
    SD_PURPLE,
    SD_BLUE,
    SD_RED,
    SD_GREEN,
    SD_YELLOW,
    SD_CYAN,
    SD_MAGENTA,
    SD_ORANGE,
    SD_GRAY
};
/**
 * @brief The TYPESHAPE enum, the different shapes.
 */
enum TYPESHAPE{
    SQUARE,
    CIRCLE,
    TRIANGLE
};


/**
 * @brief getShapeQStr, get the type fo shape as a string.
 * @return QString.
 */
inline QString getShapeQStr(const TYPESHAPE& shape)
{
    switch(shape)
    {
    case SQUARE: return "Square";
    case CIRCLE: return "Circle";
    case TRIANGLE: return "Triangle";
    }
}

inline std::string getColorStr(const SD_COLOR& color)
{
    switch(color)
    {
    case SD_PURPLE: return "PURPLE"; break;
    case SD_BLUE: return "BLUE";break;
    case SD_RED: return "RED";break;
    case SD_GREEN: return "GREEN";break;
    case SD_YELLOW: return "YELLOW";break;
    case SD_CYAN: return "CYAN";break;
    case SD_MAGENTA: return "MAGNETA";break;
    case SD_ORANGE: return "ORANGE";break;
    }
}

inline SD_COLOR getColor(const unsigned char ch)
{
    switch(ch)
    {
    case 'P': return SD_PURPLE; break;
    case 'B': return SD_BLUE; break;
    case 'R': return SD_RED; break;
    case 'G': return SD_GREEN; break;
    case 'Y': return SD_YELLOW; break;
    case 'C': return SD_CYAN; break;
    case 'M': return SD_MAGENTA; break;
    case 'O': return SD_ORANGE; break;
    }
    return SD_GRAY;
}



#endif // SHAPEDEFINITIONS_H
