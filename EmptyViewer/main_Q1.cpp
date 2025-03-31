#include <Windows.h>
#include <iostream>
#include <GL/glew.h>
#include <GL/GL.h>
#include <GL/freeglut.h>

#define GLFW_INCLUDE_GLU
#define GLFW_DLL
#include <GLFW/glfw3.h>
#include <vector>
#include <limits>
#include <cmath>

#define GLM_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

using namespace glm;

int Width = 512;
int Height = 512;
std::vector<float> OutputImage;

struct Ray {
    vec3 origin;
    vec3 direction;
    Ray(const vec3& o, const vec3& d) : origin(o), direction(normalize(d)) {}
};

class Surface {
public:
    vec3 ka, kd, ks;
    float specPower;
    Surface(const vec3& a, const vec3& d, const vec3& s, float sp)
        : ka(a), kd(d), ks(s), specPower(sp) {}

    virtual bool intersect(const Ray& ray, float& t, vec3& normal) const = 0;
    virtual vec3 getPoint(float t, const Ray& ray) const = 0;
    virtual ~Surface() {}
};

class Sphere : public Surface {
public:
    vec3 center;
    float radius;

    Sphere(const vec3& c, float r, const vec3& a, const vec3& d, const vec3& s, float sp)
        : Surface(a, d, s, sp), center(c), radius(r) {}

    bool intersect(const Ray& ray, float& t, vec3& normal) const override {
        vec3 p = ray.origin - center;
        vec3 d = ray.direction;

        float t_m = -dot(p, d);
        float p_dot = dot(p, d);
        float p_len2 = dot(p, p);
        float delta2 = p_dot * p_dot - p_len2 + radius * radius;

        if (delta2 < 0.0f) return false;

        float delta = std::sqrt(delta2);
        float t0 = t_m - delta;
        float t1 = t_m + delta;

        if (t0 > 0.001f) t = t0;
        else if (t1 > 0.001f) t = t1;
        else return false;

        vec3 hitPoint = ray.origin + t * ray.direction;
        normal = normalize(hitPoint - center);
        return true;
    }

    vec3 getPoint(float t, const Ray& ray) const override {
        return ray.origin + t * ray.direction;
    }
};

class Plane : public Surface {
public:
    vec3 normalVec;
    float d;

    Plane(const vec3& n, float dVal, const vec3& a, const vec3& kd, const vec3& s, float sp)
        : Surface(a, kd, s, sp), normalVec(normalize(n)), d(dVal) {}

    bool intersect(const Ray& ray, float& t, vec3& normal) const override {
        float denom = dot(normalVec, ray.direction);
        if (abs(denom) < 1e-5f) return false;

        t = -(dot(normalVec, ray.origin) + d) / denom;
        if (t < 0.001f) return false;

        normal = normalVec;
        return true;
    }

    vec3 getPoint(float t, const Ray& ray) const override {
        return ray.origin + t * ray.direction;
    }
};

class Camera {
public:
    vec3 e = vec3(0, 0, 0);
    vec3 u = vec3(1, 0, 0);
    vec3 v = vec3(0, 1, 0);
    vec3 w = vec3(0, 0, 1);
    float l = -0.1f, r = 0.1f, b = -0.1f, t = 0.1f, d = 0.1f;
    int nx = 512, ny = 512;

    Ray getRay(int ix, int iy) const {
        float u_s = l + (r - l) * (ix + 0.5f) / nx;
        float v_s = b + (t - b) * (iy + 0.5f) / ny;
        vec3 dir = normalize(u_s * u + v_s * v - d * w);
        return Ray(e, dir);
    }
};

class Scene {
public:
    std::vector<Surface*> objects;
    vec3 lightPos = vec3(-4, 4, -3);
    vec3 lightColor = vec3(1.0f);

    void addObject(Surface* obj) {
        objects.push_back(obj);
    }

    vec3 trace(const Ray& ray) const {
        float closestT = FLT_MAX;
        Surface* hit = nullptr;
        vec3 normal;

        for (auto obj : objects) {
            float t;
            vec3 n;
            if (obj->intersect(ray, t, n) && t < closestT) {
                closestT = t;
                hit = obj;
                normal = n;
            }
        }

        if (hit) {
            vec3 point = hit->getPoint(closestT, ray);
            vec3 L = normalize(lightPos - point);
            vec3 V = normalize(-ray.direction);
            vec3 R = reflect(-L, normal);

            float diff = std::max(dot(normal, L), 0.0f);
            float spec = pow(std::max(dot(R, V), 0.0f), hit->specPower);

            vec3 ambient = hit->ka * lightColor;
            vec3 diffuse = diff * hit->kd * lightColor;
            vec3 specular = spec * hit->ks * lightColor;

            return clamp(ambient + diffuse + specular, 0.0f, 1.0f);
        }
        return vec3(0);
    }

    ~Scene() {
        for (auto obj : objects) delete obj;
    }
};

void render() {
    Camera camera;
    Scene scene;

    scene.addObject(new Plane(vec3(0, 1, 0), 2.0f, vec3(0.2f), vec3(1, 1, 1), vec3(0, 0, 0), 0.0f));
    scene.addObject(new Sphere(vec3(-4, 0, -7), 1.0f, vec3(0.2f, 0, 0), vec3(1, 0, 0), vec3(0, 0, 0), 0.0f));
    scene.addObject(new Sphere(vec3(0, 0, -7), 2.0f, vec3(0, 0.2f, 0), vec3(0, 0.5f, 0), vec3(0.5f), 32.0f));
    scene.addObject(new Sphere(vec3(4, 0, -7), 1.0f, vec3(0, 0, 0.2f), vec3(0, 0, 1), vec3(0, 0, 0), 0.0f));

    OutputImage.clear();
    for (int j = 0; j < Height; ++j) {
        for (int i = 0; i < Width; ++i) {
            Ray ray = camera.getRay(i, j);
            vec3 color = scene.trace(ray);
            OutputImage.push_back(color.r);
            OutputImage.push_back(color.g);
            OutputImage.push_back(color.b);
        }
    }
}

void resize_callback(GLFWwindow*, int nw, int nh) {
    Width = nw;
    Height = nh;
    glViewport(0, 0, nw, nh);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, (double)Width, 0.0, (double)Height, 1.0, -1.0);
    OutputImage.reserve(Width * Height * 3);
    render();
}

int main(int argc, char* argv[]) {
    GLFWwindow* window;
    if (!glfwInit()) return -1;

    window = glfwCreateWindow(Width, Height, "Ray Tracing", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glfwSetFramebufferSizeCallback(window, resize_callback);
    resize_callback(NULL, Width, Height);

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawPixels(Width, Height, GL_RGB, GL_FLOAT, &OutputImage[0]);
        glfwSwapBuffers(window);
        glfwPollEvents();

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS ||
            glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GL_TRUE);
        }
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}