#include <cmath>
#include <iostream>
#include <fstream>
#include <ctime>
#include <chrono>

//везде используем тип float, а не double, т. к. float менее точен и
//обрабатывается быстрее

//вычисления в компьютере неточные, поэтому вместо прямого сравнения чисел с плавающими точками
//их разница сравнивается с некоторым маленьким числом
float epsilon = 0.0001f;

float tMin = 0.0f;
float tMax = 1000000.0f;

class Color {
public:
    int red;
    int green;
    int blue;

    Color(int _red, int _green, int _blue) {
        red = _red;
        green = _green;
        blue = _blue;
    }

    Color() {
        red = 0;
        green = 0;
        blue = 0;
    }

    Color(const Color &colorCopy) {
        red = colorCopy.red;
        green = colorCopy.green;
        blue = colorCopy.blue;
    }

    ~Color() = default;

    Color operator+(const Color &colorOperand) const {
        Color sum;
        sum.red = red + colorOperand.red;
        sum.green = green + colorOperand.green;
        sum.blue = blue + colorOperand.blue;
        return sum;
    }

    Color operator*(float operand) const {
        Color mult;
        mult.red = (int) ((float) (red) * operand);
        mult.green = (int) ((float) (green) * operand);
        mult.blue = (int) ((float) (blue) * operand);
        return mult;
    }
};

class Vector { //вектор или точка в пространстве
public:
    float x;
    float y;
    float z;

    Vector(float _x, float _y, float _z) {
        x = _x;
        y = _y;
        z = _z;
    }

    Vector() {
        x = 0.0;
        y = 0.0;
        z = 0.0;
    }

    Vector(const Vector &vectorCopy) {
        x = vectorCopy.x;
        y = vectorCopy.y;
        z = vectorCopy.z;
    }

    ~Vector() = default;

    void normalize() { //нормирование вектора
        float norm = std::sqrt(x * x + y * y + z * z);
        x = x / norm;
        y = y / norm;
        z = z / norm;
    }

    float Length() const { //длина вектора
        return std::sqrt(x * x + y * y + z * z);
    }

    Vector operator+(const Vector &vectorOperand) const {
        Vector sum;
        sum.x = x + vectorOperand.x;
        sum.y = y + vectorOperand.y;
        sum.z = z + vectorOperand.z;
        return sum;
    }

    Vector operator-(const Vector &vectorOperand) const {
        Vector dec;
        dec.x = x - vectorOperand.x;
        dec.y = y - vectorOperand.y;
        dec.z = z - vectorOperand.z;
        return dec;
    }

    Vector operator*(float operand) const {
        Vector mult;
        mult.x = x * operand;
        mult.y = y * operand;
        mult.z = z * operand;
        return mult;
    }
};

float CreateScalarVector(const Vector &operandA, const Vector &operandB) { //скалярное произведение
    return operandA.x * operandB.x + operandA.y * operandB.y + operandA.z * operandB.z;
}

//протяжённый источник освещения, представлен отрезком, разбитым на точки
class Light {
public:
    Vector leftPoint; //начало источника
    Vector rightPoint; //конец источника
    float intensity; //интенсивность света
    int stepsValue = 25; //количество разбиений сетки источника
    Vector *lightPoints; //массив для точек из сетки

    Light(const Vector &_leftPoint, const Vector &_rightPoint, float _intensity) {
        leftPoint = _leftPoint;
        rightPoint = _rightPoint;
        intensity = _intensity;
        lightPoints = new Vector[stepsValue];
        float t = 0.0f; //идём по сетке, записываем координаты точек
        for (int i = 0; i < stepsValue; i++) {
            lightPoints[i] = leftPoint + (rightPoint - leftPoint) * t;
            t += 1 / ((float) (stepsValue));
        }
    }

    Light(const Light &lightCopy) {
        leftPoint = lightCopy.leftPoint;
        rightPoint = lightCopy.rightPoint;
        intensity = lightCopy.intensity;
        stepsValue = lightCopy.stepsValue;
        lightPoints = new Vector[stepsValue];
        for (int i = 0; i < stepsValue; i++) {
            lightPoints[i] = lightCopy.lightPoints[i];
        }
    }

    ~Light() {
        delete[] lightPoints;
    }

    Vector getLightPoints(int i) const {
        return lightPoints[i];
    }

    float getIntensity() const {
        return intensity;
    }
};

class GraphicClass { //родитель всех графических объектов
public:
    Color color;
    int specular = 0; //коэффициент бликов
    float reflective = 0; //коэффициент отражения
    float gloss = 0; //коэффициент шероховатости (для нечётких отражений)

    GraphicClass() = default;

    virtual ~GraphicClass() = default;

    Color getColor() const {
        return color;
    }

    float getGloss() const {
        return gloss;
    }

    //проверка на пересечение с объектом. Возвращает параметр t для уравнения луча, чтобы найти точку пересечения.
    //cameraPosition - позиция камеры (начальный вектор для луча), ray - направление луча
    virtual float intersect(Vector &cameraPosition, Vector &ray) = 0;

    //возвращает нормаль к поверхности объекта в заданной точке onSurfacePosition
    virtual Vector getNormal(Vector &onSurfacePosition) const = 0;

    int getSpecular() const {
        return specular;
    }

    float getReflective() const {
        return reflective;
    }
};

class Sphere : public GraphicClass {
protected:
    float radius;
    Vector centerPosition; //центр сферы

public:
    Sphere(float _radius, int _specular, const Color &_color, const Vector &_position, float _reflective,
           float _gloss) {
        radius = _radius;
        specular = _specular;
        color = _color;
        centerPosition = _position;
        reflective = _reflective;
        gloss = _gloss;
    }

    ~Sphere() override = default;

    Vector getNormal(Vector &onSurfacePosition) const override {
        return onSurfacePosition - centerPosition;
    }

    //проверка на пересечение луча со сферой - решение квадратного уравнения
    float intersect(Vector &cameraPosition, Vector &ray) override {
        Vector vectorToCenter = cameraPosition - centerPosition;

        float cof1 = CreateScalarVector(ray, ray);
        float cof2 = (2 * CreateScalarVector(vectorToCenter, ray));
        float cof3 = CreateScalarVector(vectorToCenter, vectorToCenter) - radius * radius;
        float discr = cof2 * cof2 - 4 * cof1 * cof3;

        //Если из любой функции intersect вернуть -1, то пересечения нет
        if (discr < 0) {
            return -1;
        }

        //нам нужно вернуть только меньшее из значений t
        float t = (-cof2 + std::sqrt(discr)) / (2 * cof1);
        if (t > (-cof2 - std::sqrt(discr)) / (2 * cof1)) {
            t = (-cof2 - std::sqrt(discr)) / (2 * cof1);
        }

        return t;
    }
};

class ImageData {
protected:
    int width;
    int height;
    Color *pixels; //указатель на массив пикселей

public:
    ImageData(int w, int h) {
        width = w;
        height = h;
        int bufSize = w * h;
        pixels = new Color[bufSize];
        Color white(255, 255, 255);
        for (int i = 0; i < bufSize; i++) {
            pixels[i] = white;
        }
    }

    ImageData(const ImageData &a) {
        width = a.width;
        height = a.height;
        int bufSize = width * height;
        pixels = new Color[bufSize];
        for (int i = 0; i < bufSize; i++) {
            pixels[i] = a.pixels[i];
        }
    }

    ~ImageData() {
        delete[] pixels;
    }

    int getWidth() const {
        return width;
    }

    int getHeight() const {
        return height;
    }

    void putPixel(const Color &color, int pixelPointer) {
        pixels[pixelPointer] = color;
    }

    //запись в файл
    void writeInFile() {

        std::cout << "\nWriting in file\n";

        //вставьте удобный Вам путь, чтобы сохранить файл
        std::ofstream fileForPPM(R"(D:\Users\agule\CLionProjects\cmc-mashgraph-2\cmake-build-debug\picture.ppm)");

        //добавляем в файл заголовок для формата .ppm
        fileForPPM << "P3" << std::endl;
        fileForPPM << width << " " << height << std::endl;
        fileForPPM << "255" << std::endl;

        for (int i = 0; i < width * height; i++) {
            fileForPPM << pixels[i].red << " "
                       << pixels[i].green << " "
                       << pixels[i].blue << std::endl;
        }

        fileForPPM.close();

        std::cout << "\nReady!\n";
    }
};

//проверка для каждого объекта в сцене, есть ли с ним пересчение.
//если пересечение есть, то возвращаем указатель на объект,
//который ближе всего
GraphicClass *checkForAnyObject(float &t, GraphicClass **array, Vector cameraPosition, Vector ray) {
    GraphicClass *closestObject = nullptr;
    float tempT;
    float closestT = 1000000;

    for (int i = 0; i < 6; i++) {
        tempT = array[i]->intersect(cameraPosition, ray);
        if ((tempT - tMin > epsilon) && (tMax - tempT > epsilon) && (closestT - tempT > epsilon)) {
            closestT = tempT;
            closestObject = array[i];
        }
    }

    t = closestT;
    return closestObject;
}

Vector ReflectRay(const Vector &ray, const Vector &normal) { //отражает луч вдоль нормали
    return normal * (2 * CreateScalarVector(normal, ray)) - ray;
}

//расчёт освещённости для пикселя, возвращает интенсивность отражённого в точке onSurfacePosition света в переменной intensity
float ComputeLight(Light &light, GraphicClass **array, const Vector &onSurfacePosition, const Vector &normal,
                   const Vector &minusRay,
                   GraphicClass *closestObject) {
    float n, r, intensity = 0;
    float shadowT;
    Vector lightDirect;
    GraphicClass *shadowObj;

    //расчёт освещённости идёт для каждой позиции в сетке, на которую разбит протяжённый источник света
    for (int i = 0; i < light.stepsValue; i++) {
        lightDirect = light.getLightPoints(i) - onSurfacePosition; //направление света от одной точки
        lightDirect.normalize();

        //проверка, есть ли тень
        shadowObj = checkForAnyObject(shadowT, array, onSurfacePosition, lightDirect);
        if (shadowObj != nullptr) {
            continue;
        }

        //расчёт, сколько света падает в точку
        n = CreateScalarVector(normal, lightDirect);
        if (n > epsilon) {
            intensity += light.getIntensity() * n / (normal.Length() * lightDirect.Length());
        }

        //расчёт блеска в зависимости от свойств объекта
        if ((closestObject->getSpecular() != 0)) {
            Vector reflection = ReflectRay(lightDirect, normal);
            reflection.normalize();
            r = CreateScalarVector(reflection, minusRay);
            if (r > epsilon) {
                intensity += light.getIntensity() * pow((r / (reflection.Length() * minusRay.Length())), closestObject->getSpecular());
            }
        }
    }

    //если значение intensity окажется слишком большим,
    //то появятся засвеченные области красного цвета
    if (intensity > 0.8) {
        intensity = 0.8f;
    }

    return intensity;
}

float RandFloat() {
    return rand() / (float) (RAND_MAX);
}

Vector RandomVector() { //случайные единичные векторы
    Vector temp(RandFloat(), RandFloat(), RandFloat());
    temp.normalize();
    return temp;
}

//функция для расчёта цвета в точке, направление луча передаётся в функцию
Color CalculateColor(Vector &cameraPosition, Vector &ray, GraphicClass **array, Light &light, float depth) {
    float closestT;
    Color backgroundColor(0, 0, 0);
    int rays = 3; //количество лучей, для которых считаем среднее значение цвета.
    //Это создаёт нечёткие отражения.
    Color tempColor, reflectedColor, totalReflectedColor, totalColor;
    Vector reflectedRay;
    GraphicClass *closestObject;
    Vector null;
    float ambientLight = 0.2f; //интенсивность окружающего света

    //ищем ближайший объект, с которым есть пересчение
    closestObject = checkForAnyObject(closestT, array, cameraPosition, ray);

    //если луч ни с чем не пересекается, возвращаем цвет фона
    if (closestObject == nullptr) {
        return backgroundColor;
    }

    //нормаль для работы с отражениями
    Vector onSurfacePosition = cameraPosition + ray * closestT;
    Vector normal = closestObject->getNormal(onSurfacePosition);
    normal.normalize();

    //расчёт света в вызове ComputeLight, запоминаем цвет
    tempColor = closestObject->getColor() * (ComputeLight(light, array, onSurfacePosition, normal, null - ray, closestObject) + ambientLight);

    //depth - рекурсивный параметр. Проверяем его, чтобы понять, когда нужная глубина
    //отражений достигнута, и можно больше не вызывать рекурсию
    if ((depth <= 0) || (closestObject->getReflective() < 0)) {
        return tempColor;
    }

    //расчитываем цвет отражения, если коэффициент отражения > 0
    if (closestObject->getReflective() != 0.0f) {
        for (int i = 0; i < rays; i++) {
            reflectedRay = ReflectRay(null - ray, normal) + RandomVector() * closestObject->getGloss();

            //рекурсивный вызов
            reflectedColor = CalculateColor(onSurfacePosition, reflectedRay, array, light, depth - (float) (1));

            //считаем среднее значение цвета
            totalReflectedColor.red += (int) (reflectedColor.red * 1.0 / rays);
            totalReflectedColor.green += (int) (reflectedColor.green * 1.0 / rays);
            totalReflectedColor.blue += (int) (reflectedColor.blue * 1.0 / rays);
        }
    }

    totalColor = tempColor * (1 - closestObject->getReflective()) + totalReflectedColor * (closestObject->getReflective());

    return totalColor;
}

void TypeProgress(int y) {
    if (y % 100 == 0) {
        std::cout << "*" << std::endl;
    }
}

//основная функция, которая вызывает рендер и записывает результат в изображение
void Render(ImageData &image, GraphicClass **array, Light &light) {
    int pixelPointer = 0;
    float anti = 0.25f; //параметр антиалиасинга. Очень сильно влияет на производительность.
    Color totalColor, tempColor;
    float red, green, blue;
    Vector cameraPosition; //позиция камеры
    float depth = 5.0f; //глубина отражений (сколько раз отражаем луч)

    //расстояние от камеры до воображаемого холста
    float distance = (float) (image.getWidth() + image.getHeight()) / 2;

    //начало рендера, шаг - один пиксель
    for (int y = -(image.getHeight() / 2); y < image.getHeight() / 2; y++) {

        TypeProgress(y);

        for (int x = -(image.getWidth() / 2); x < image.getWidth() / 2; x++) {

            red = 0.0f;
            green = 0.0f;
            blue = 0.0f;

            //антиалиасинг. Дробим координаты на сетку, в каждой точке которой считаем луч.
            //итоговый цвет пикселя = среднее значение всех собранных цветов
            //anti = 0.5f считает 4 луча на пиксель
            for (float newY = (float) (y); newY < (float) (y + 1); newY += anti) {
                for (float newX = (float) (x); newX < (float) (x + 1); newX += anti) {

                    Vector newRay(newX, -newY, distance);
                    newRay.normalize();

                    //выпускаем луч, считаем цвет на пиксель
                    tempColor = CalculateColor(cameraPosition, newRay, array, light, depth);

                    red += tempColor.red * anti * anti;
                    green += tempColor.green * anti * anti;
                    blue += tempColor.blue * anti * anti;
                }
            }

            totalColor.red = (int) (red);
            totalColor.green = (int) (green);
            totalColor.blue = (int) (blue);

            //запись в массив пикселей
            image.putPixel(totalColor, pixelPointer);

            pixelPointer++;
        }
    }
}

GraphicClass **CreateScene() { //описание сцены
    //используемые в сцене цвета
    Color white(255, 255, 255);
    Color sand(255, 255, 224);
    Color purple(148, 0, 210);

    //пол - гигантская сфера
    Vector floorPosition(0.0f, -5001.0f, 0.0f);
    Sphere floorSphere(5000.0f, 1000, sand, floorPosition, 0.0f, 0.0f);

    //зеркальная сфера
    Vector mirrorPosition(2.5f, 1.0f, 10.0f);
    Sphere mirrorSphere(1.9f, 1000, white, mirrorPosition, 0.8f, 0.0f);

    //шероховатая сфера
    Vector roughPosition(-2.5f, 1.0f, 10.0f);
    Sphere roughSphere(1.9f, 1000, white, roughPosition, 0.5f, 0.1f);

    //зелёные сферки для наглядности
    Vector pos1(-1.0f, -0.5f, 5.0f);
    Sphere sphere1(0.3f, 500, purple, pos1, 0.1f, 0.0f);

    Vector pos2(0.0f, -0.5f, 5.0f);
    Sphere sphere2(0.3f, 500, purple, pos2, 0.1f, 0.0f);

    Vector pos3(1.0f, -0.5f, 5.0f);
    Sphere sphere3(0.3f, 500, purple, pos3, 0.1f, 0.0f);

    auto **arrayOfObjects = new GraphicClass *[6];
    arrayOfObjects[0] = new Sphere(floorSphere);
    arrayOfObjects[1] = new Sphere(mirrorSphere);
    arrayOfObjects[2] = new Sphere(roughSphere);
    arrayOfObjects[3] = new Sphere(sphere1);
    arrayOfObjects[4] = new Sphere(sphere2);
    arrayOfObjects[5] = new Sphere(sphere3);

    return arrayOfObjects;
}

int main() {
    ImageData image(1500, 1000);
    auto **array = CreateScene();
    auto now = std::chrono::system_clock::now();
    std::time_t start_time = std::chrono::system_clock::to_time_t(now);
    std::cout << "Start to render " << std::ctime(&start_time) << std::endl;

    //свет для сцены
    Vector leftLight(-100.0f, 10.0f, 0.0f);
    Vector rightLight(100.0f, 10.0f, 0.0f);
    Light light(leftLight, rightLight, 0.05f);

    //рендер, запись его в изображение
    Render(image, array, light);
    image.writeInFile();
    now = std::chrono::system_clock::now();
    start_time = std::chrono::system_clock::to_time_t(now);
    std::cout << "End to render " << std::ctime(&start_time) << std::endl;
    return 0;
}