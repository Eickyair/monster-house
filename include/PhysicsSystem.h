#ifndef PHYSICS_SYSTEM_H
#define PHYSICS_SYSTEM_H

#include <algorithm>

/**
 * @brief Simulación de salto lunar
 */
class PhysicsSystem {
private:
    const float LUNAR_GRAVITY = 1.62f;
    const float ASTRONAUT_MASS = 100.0f;
    const float JUMP_INITIAL_VELOCITY = 3.0f;

    bool isJumping;
    bool isGrounded;
    float jumpStartTime;
    float currentTime;
    float groundLevel;

public:
    PhysicsSystem()
        : isJumping(false), isGrounded(true), jumpStartTime(0.0f),
          currentTime(0.0f), groundLevel(0.0f) {
    }

    void initiateJump() {
        if (isGrounded) {
            isJumping = true;
            isGrounded = false;
            jumpStartTime = currentTime;
        }
    }

    void update(float deltaTime) {
        currentTime += deltaTime;

        if (isJumping) {
            float t = getJumpTime();
            float y = (JUMP_INITIAL_VELOCITY * t) - (0.5f * LUNAR_GRAVITY * t * t);

            // Verificar si tocó el suelo
            if (y <= 0.0f) {
                isGrounded = true;
                isJumping = false;
                jumpStartTime = 0.0f;
            }
        }
    }

    /**
     * @brief Calcula el desplazamiento vertical actual (altura del salto).
     */
    float getCurrentVerticalDisplacement() const {
        if (!isJumping) {
            return 0.0f;
        }

        float t = getJumpTime();
        float v0 = JUMP_INITIAL_VELOCITY;
        float g = LUNAR_GRAVITY;

        // Ecuación de cinemática: y = v0*t - (1/2)*g*t²
        float displacement = (v0 * t) - (0.5f * g * t * t);

        // No permitir desplazamiento negativo (no atravesar el suelo)
        return std::max(0.0f, displacement);
    }

    float getJumpTime() const {
        return isJumping ? (currentTime - jumpStartTime) : 0.0f;
    }

    // Getters
    bool getIsJumping() const { return isJumping; }
    bool getIsGrounded() const { return isGrounded; }
    float getInitialVelocity() const { return JUMP_INITIAL_VELOCITY; }
    float getLunarGravity() const { return LUNAR_GRAVITY; }
    float getAstronautMass() const { return ASTRONAUT_MASS; }
    float getGroundLevel() const { return groundLevel; }

    // Setters
    void setGroundLevel(float level) { groundLevel = level; }
    void reset() {
        isJumping = false;
        isGrounded = true;
        jumpStartTime = 0.0f;
    }
};

#endif // PHYSICS_SYSTEM_H
