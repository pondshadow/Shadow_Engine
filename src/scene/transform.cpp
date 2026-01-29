#include "../scene/transform.h"

Transform::Transform()
{
    reset();
}

void Transform::reset()
{
    position = glm::vec3(0.0f, 0.0f, 0.0f);
    rotation = glm::vec3(0.0f, 0.0f, 0.0f);
    scale    = glm::vec3(1.0f, 1.0f, 1.0f);
}

glm::mat4 Transform::get_model_matrix() const
{
    glm::mat4 model = glm::mat4(1.0f);

    // 位移 (Translation)
    model = glm::translate(model, position);

    // 旋转 (Rotation) - 欧拉角 (Z-Y-X 顺序或其他顺序，这里简单起见按 X->Y->Z 累乘)
    // 注意：glm::rotate 接收弧度
    // 为了防止万向节死锁，成熟引擎会用四元数 (Quaternion)，但目前欧拉角足够了
    model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

    // 缩放 (Scale)
    model = glm::scale(model, scale);

    return model;
}