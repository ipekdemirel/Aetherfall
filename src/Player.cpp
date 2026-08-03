#include "Player.h"

#include <cctype>
#include <cstddef>
#include <cmath>
#include <cstring>
#include <vector>

#include <raymath.h>

namespace
{
    struct CharacterAssets
    {
        Model model{};
        Model displayModel{};
        ModelAnimation* animations = nullptr;

        int animationCount = 0;
        int idleAnimation = -1;
        int walkAnimation = -1;
        int runAnimation = -1;
        int attackAnimation = -1;
        int rollAnimation = -1;

        std::vector<unsigned char> animationUsable;
        std::vector<Mesh> safeAnimationMeshes;
        std::vector<Mesh> displayMeshes;
        std::vector<int> displayMeshMaterials;

        float scale = 1.0f;
        float groundOffset = 0.0f;

        bool loadAttempted = false;
        bool modelLoaded = false;
        bool embeddedSwordHidden = false;
    };

    CharacterAssets& GetCharacterAssets()
    {
        static CharacterAssets assets;
        return assets;
    }

    void BuildDisplayModelWithoutEmbeddedSword(
        CharacterAssets& assets
    )
    {
        assets.displayModel =
            assets.model;

        assets.displayMeshes.clear();
        assets.displayMeshMaterials.clear();
        assets.embeddedSwordHidden = false;

        // Quaternius Warrior.gltf mesh sirasi:
        // 0 Body, 1 Face, 2 ShoulderPad.L,
        // 3 Warrior_Sword, 4 ShoulderPad.R.
        //
        // Modelin kendi kilici raylib 5.5'te ele bagli
        // animasyon alamadigi icin sabit kaliyor. Yalnizca
        // 3 numarali kilici gorunur modelden cikariyoruz.
        // Diger parcalar V3'teki gibi tek model olarak cizilir;
        // boylece vucut parcalari birbirine girmez.
        if (
            assets.model.meshCount != 5 ||
            assets.model.meshes == nullptr ||
            assets.model.meshMaterial == nullptr
            )
        {
            return;
        }

        assets.displayMeshes.reserve(4);
        assets.displayMeshMaterials.reserve(4);

        for (
            int meshIndex = 0;
            meshIndex < assets.model.meshCount;
            ++meshIndex
            )
        {
            if (meshIndex == 3)
            {
                continue;
            }

            assets.displayMeshes.push_back(
                assets.model.meshes[meshIndex]
            );

            assets.displayMeshMaterials.push_back(
                assets.model.meshMaterial[meshIndex]
            );
        }

        assets.displayModel.meshCount =
            static_cast<int>(
                assets.displayMeshes.size()
                );

        assets.displayModel.meshes =
            assets.displayMeshes.data();

        assets.displayModel.meshMaterial =
            assets.displayMeshMaterials.data();

        assets.embeddedSwordHidden = true;
    }

    bool ContainsIgnoreCase(
        const char* text,
        const char* searchText
    )
    {
        if (
            text == nullptr ||
            searchText == nullptr ||
            searchText[0] == '\0'
            )
        {
            return false;
        }

        const std::size_t textLength =
            std::strlen(text);

        const std::size_t searchLength =
            std::strlen(searchText);

        if (searchLength > textLength)
        {
            return false;
        }

        for (
            std::size_t start = 0;
            start + searchLength <= textLength;
            ++start
            )
        {
            bool matches = true;

            for (
                std::size_t index = 0;
                index < searchLength;
                ++index
                )
            {
                const unsigned char textCharacter =
                    static_cast<unsigned char>(
                        text[start + index]
                        );

                const unsigned char searchCharacter =
                    static_cast<unsigned char>(
                        searchText[index]
                        );

                if (
                    std::tolower(textCharacter) !=
                    std::tolower(searchCharacter)
                    )
                {
                    matches = false;
                    break;
                }
            }

            if (matches)
            {
                return true;
            }
        }

        return false;
    }

    int FindAnimation(
        ModelAnimation* animations,
        int animationCount,
        const char* namePart
    )
    {
        if (
            animations == nullptr ||
            animationCount <= 0
            )
        {
            return -1;
        }

        for (
            int animationIndex = 0;
            animationIndex < animationCount;
            ++animationIndex
            )
        {
            if (
                ContainsIgnoreCase(
                    animations[animationIndex].name,
                    namePart
                )
                )
            {
                return animationIndex;
            }
        }

        return -1;
    }

#if !defined(RAYLIB_VERSION_MAJOR) || (RAYLIB_VERSION_MAJOR < 6)
    bool IsMeshSafeForCpuAnimation(
        const Mesh& mesh,
        int modelBoneCount
    )
    {
        if (
            mesh.vertexCount <= 0 ||
            modelBoneCount <= 0 ||
            mesh.vertices == nullptr ||
            mesh.boneIds == nullptr ||
            mesh.boneWeights == nullptr ||
            mesh.animVertices == nullptr ||
            mesh.boneMatrices == nullptr ||
            mesh.vboId == nullptr ||
            mesh.boneCount != modelBoneCount
            )
        {
            return false;
        }

        if (
            mesh.normals != nullptr &&
            mesh.animNormals == nullptr
            )
        {
            return false;
        }

        if (mesh.vboId[0] == 0)
        {
            return false;
        }

        if (
            mesh.normals != nullptr &&
            mesh.vboId[2] == 0
            )
        {
            return false;
        }

        const int influenceCount =
            mesh.vertexCount * 4;

        for (
            int influenceIndex = 0;
            influenceIndex < influenceCount;
            ++influenceIndex
            )
        {
            if (
                mesh.boneWeights[influenceIndex] <=
                0.0f
                )
            {
                continue;
            }

            const int boneIndex =
                static_cast<int>(
                    mesh.boneIds[influenceIndex]
                    );

            if (
                boneIndex < 0 ||
                boneIndex >= modelBoneCount
                )
            {
                return false;
            }
        }

        return true;
    }

    void BuildSafeAnimationMeshList(
        CharacterAssets& assets
    )
    {
        assets.safeAnimationMeshes.clear();

        if (
            !assets.modelLoaded ||
            assets.model.meshes == nullptr ||
            assets.model.meshCount <= 0
            )
        {
            return;
        }

        assets.safeAnimationMeshes.reserve(
            static_cast<std::size_t>(
                assets.model.meshCount
                )
        );

        for (
            int meshIndex = 0;
            meshIndex < assets.model.meshCount;
            ++meshIndex
            )
        {
            const Mesh& mesh =
                assets.model.meshes[meshIndex];

            if (
                IsMeshSafeForCpuAnimation(
                    mesh,
                    assets.model.boneCount
                )
                )
            {
                // Mesh yapisi bilincli olarak sig kopyalanir.
                // Vertex/VBO verileri asil modele ait kalir.
                assets.safeAnimationMeshes.push_back(
                    mesh
                );
            }
        }
    }
#endif

    bool IsAnimationPlaybackSafe(
        const CharacterAssets& assets,
        int animationIndex
    )
    {
        if (
            !assets.modelLoaded ||
            assets.animations == nullptr ||
            animationIndex < 0 ||
            animationIndex >= assets.animationCount
            )
        {
            return false;
        }

        const ModelAnimation& animation =
            assets.animations[animationIndex];

#if defined(RAYLIB_VERSION_MAJOR) && (RAYLIB_VERSION_MAJOR >= 6)
        // raylib 6.0 animasyon sistemini yeniden tasarladı.
        // Yeni yapıda raylib'in kendi iskelet denetimi kullanılır.
        return IsModelAnimationValid(
            assets.model,
            animation
        );
#else
        // raylib 5.x UpdateModelAnimation(), eksik bir işaretçiyi
        // kendi içinde güvenli biçimde reddetmediği için önce bütün
        // gerekli model ve animasyon verileri burada kontrol edilir.
        if (
            assets.model.boneCount <= 0 ||
            assets.model.bones == nullptr ||
            assets.model.bindPose == nullptr ||
            animation.boneCount <= 0 ||
            animation.bones == nullptr ||
            animation.frameCount <= 0 ||
            animation.framePoses == nullptr
            )
        {
            return false;
        }

        for (
            int frameIndex = 0;
            frameIndex < animation.frameCount;
            ++frameIndex
            )
        {
            if (animation.framePoses[frameIndex] == nullptr)
            {
                return false;
            }
        }

        if (
            !IsModelAnimationValid(
                assets.model,
                animation
            )
            )
        {
            return false;
        }

        return !assets.safeAnimationMeshes.empty();
#endif
    }

    int FirstUsableAnimation(
        const CharacterAssets& assets
    )
    {
        for (
            int animationIndex = 0;
            animationIndex < assets.animationCount;
            ++animationIndex
            )
        {
            if (
                animationIndex <
                static_cast<int>(
                    assets.animationUsable.size()
                    ) &&
                assets.animationUsable[animationIndex] != 0
                )
            {
                return animationIndex;
            }
        }

        return -1;
    }

    int KeepUsableAnimationOrFallback(
        const CharacterAssets& assets,
        int animationIndex,
        int fallbackAnimation
    )
    {
        if (
            animationIndex >= 0 &&
            animationIndex <
            static_cast<int>(
                assets.animationUsable.size()
                ) &&
            assets.animationUsable[animationIndex] != 0
            )
        {
            return animationIndex;
        }

        return fallbackAnimation;
    }

    void LoadCharacterAssets()
    {
        CharacterAssets& assets =
            GetCharacterAssets();

        if (assets.loadAttempted)
        {
            return;
        }

        assets.loadAttempted = true;

        // Visual Studio ve CMake oyunu farklı klasörlerden
        // başlatabildiği için model birkaç güvenli yolda aranır.
        const char* modelPaths[] =
        {
            "assets/models/Warrior.gltf",
            "../assets/models/Warrior.gltf",
            "../../assets/models/Warrior.gltf",
            "../../../assets/models/Warrior.gltf",
            "../../../../assets/models/Warrior.gltf"
        };

        const char* selectedPath = nullptr;

        for (const char* path : modelPaths)
        {
            if (FileExists(path))
            {
                selectedPath = path;
                break;
            }
        }

        if (selectedPath == nullptr)
        {
            TraceLog(
                LOG_WARNING,
                "Warrior.gltf bulunamadi. "
                "assets/models klasorunu kontrol edin."
            );

            return;
        }

        assets.model =
            LoadModel(selectedPath);

        assets.modelLoaded =
            assets.model.meshCount > 0 &&
            assets.model.meshes != nullptr;

        if (!assets.modelLoaded)
        {
            TraceLog(
                LOG_WARNING,
                "Warrior.gltf yuklenemedi: %s",
                selectedPath
            );

            return;
        }

        const BoundingBox bounds =
            GetModelBoundingBox(
                assets.model
            );

        const float originalHeight =
            bounds.max.y -
            bounds.min.y;

        // Modelin boyunu oyun dünyasında yaklaşık 2.25 birime getirir.
        if (originalHeight > 0.001f)
        {
            assets.scale =
                2.25f /
                originalHeight;
        }

        assets.groundOffset =
            -bounds.min.y *
            assets.scale;

        BuildDisplayModelWithoutEmbeddedSword(
            assets
        );

        assets.animations =
            LoadModelAnimations(
                selectedPath,
                &assets.animationCount
            );

        if (
            assets.animations == nullptr ||
            assets.animationCount <= 0
            )
        {
            assets.animations = nullptr;
            assets.animationCount = 0;

            TraceLog(
                LOG_WARNING,
                "Warrior modeli yuklendi fakat animasyon bulunamadi."
            );

            return;
        }

#if !defined(RAYLIB_VERSION_MAJOR) || (RAYLIB_VERSION_MAJOR < 6)
        // raylib 5.5, iskelete bagli olmayan bir mesh'i (ornegin
        // migfer veya aksesuar) animasyon sirasinda bos boneWeights
        // verisiyle islemeye calisabiliyor. Yalnizca tam CPU
        // skinning verisi bulunan mesh'leri ayiriyoruz.
        BuildSafeAnimationMeshList(assets);

        TraceLog(
            LOG_INFO,
            "Warrior animasyon meshleri: %i/%i",
            static_cast<int>(
                assets.safeAnimationMeshes.size()
                ),
            assets.model.meshCount
        );
#endif

        assets.animationUsable.assign(
            static_cast<std::size_t>(
                assets.animationCount
                ),
            0
        );

        int usableAnimationCount = 0;

        for (
            int animationIndex = 0;
            animationIndex < assets.animationCount;
            ++animationIndex
            )
        {
            if (
                IsAnimationPlaybackSafe(
                    assets,
                    animationIndex
                )
                )
            {
                assets.animationUsable[animationIndex] = 1;
                ++usableAnimationCount;
            }
        }

        const int firstUsableAnimation =
            FirstUsableAnimation(assets);

        assets.idleAnimation =
            KeepUsableAnimationOrFallback(
                assets,
                FindAnimation(
                    assets.animations,
                    assets.animationCount,
                    "idle"
                ),
                firstUsableAnimation
            );

        assets.walkAnimation =
            KeepUsableAnimationOrFallback(
                assets,
                FindAnimation(
                    assets.animations,
                    assets.animationCount,
                    "walk"
                ),
                assets.idleAnimation
            );

        assets.runAnimation =
            KeepUsableAnimationOrFallback(
                assets,
                FindAnimation(
                    assets.animations,
                    assets.animationCount,
                    "run"
                ),
                assets.walkAnimation
            );

        assets.attackAnimation =
            FindAnimation(
                assets.animations,
                assets.animationCount,
                "sword_attack"
            );

        if (assets.attackAnimation < 0)
        {
            assets.attackAnimation =
                FindAnimation(
                    assets.animations,
                    assets.animationCount,
                    "attack"
                );
        }

        assets.attackAnimation =
            KeepUsableAnimationOrFallback(
                assets,
                assets.attackAnimation,
                assets.idleAnimation
            );

        assets.rollAnimation =
            KeepUsableAnimationOrFallback(
                assets,
                FindAnimation(
                    assets.animations,
                    assets.animationCount,
                    "roll"
                ),
                assets.runAnimation
            );

        TraceLog(
            LOG_INFO,
            "Warrior modeli yuklendi. "
            "Guvenli animasyon: %i/%i",
            usableAnimationCount,
            assets.animationCount
        );

        if (usableAnimationCount <= 0)
        {
            TraceLog(
                LOG_WARNING,
                "Warrior animasyon iskeleti raylib ile uyumlu degil. "
                "Model guvenli sabit pozda gosterilecek."
            );
        }
    }

    Vector3 AddScaled(
        Vector3 origin,
        Vector3 direction,
        float amount
    )
    {
        return Vector3{
            origin.x + direction.x * amount,
            origin.y + direction.y * amount,
            origin.z + direction.z * amount
        };
    }

    Vector3 RotateOnGround(
        Vector3 direction,
        float angle
    )
    {
        const float cosine =
            std::cos(angle);

        const float sine =
            std::sin(angle);

        return Vector3{
            direction.x * cosine -
                direction.z * sine,
            0.0f,
            direction.x * sine +
                direction.z * cosine
        };
    }

    void DrawFallbackCharacter(
        Vector3 position,
        Vector3 facingDirection
    )
    {
        const Vector3 bodyCenter{
            position.x,
            position.y + 0.15f,
            position.z
        };

        DrawCylinderEx(
            Vector3{
                bodyCenter.x,
                bodyCenter.y - 0.65f,
                bodyCenter.z
            },
            Vector3{
                bodyCenter.x,
                bodyCenter.y + 0.50f,
                bodyCenter.z
            },
            0.42f,
            0.30f,
            8,
            Color{ 36, 92, 130, 255 }
        );

        DrawSphere(
            Vector3{
                bodyCenter.x,
                bodyCenter.y + 0.78f,
                bodyCenter.z
            },
            0.30f,
            Color{ 174, 190, 202, 255 }
        );

        DrawCylinderEx(
            AddScaled(
                bodyCenter,
                facingDirection,
                0.35f
            ),
            AddScaled(
                bodyCenter,
                facingDirection,
                1.45f
            ),
            0.08f,
            0.025f,
            8,
            SKYBLUE
        );
    }
}

Player::Player()
    : position{ 0.0f, 1.0f, 0.0f },
    facingDirection{ 0.0f, 0.0f, -1.0f },
    walkSpeed(6.0f),
    runSpeed(9.0f),
    dashSpeed(20.0f),
    dashDuration(0.18f),
    dashTimer(0.0f),
    dashCooldown(0.8f),
    dashCooldownTimer(0.0f),
    isDashing(false),
    isMoving(false),
    isRunning(false),
    effectTime(0.0f),
    modelAnimationFrame(0.0f),
    currentModelAnimation(-1),
    health(100),
    maxHealth(100),
    armor(0.0f)
{
    LoadCharacterAssets();

    const CharacterAssets& assets =
        GetCharacterAssets();

    currentModelAnimation =
        assets.idleAnimation;
}

void Player::Update(float deltaTime)
{
    weapon.Update(deltaTime);

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        weapon.StartAttack();
    }

    if (dashCooldownTimer > 0.0f)
    {
        dashCooldownTimer -= deltaTime;

        if (dashCooldownTimer < 0.0f)
        {
            dashCooldownTimer = 0.0f;
        }
    }

    Vector3 movement{ 0.0f, 0.0f, 0.0f };

    if (IsKeyDown(KEY_W))
    {
        movement.z -= 1.0f;
    }

    if (IsKeyDown(KEY_S))
    {
        movement.z += 1.0f;
    }

    if (IsKeyDown(KEY_A))
    {
        movement.x -= 1.0f;
    }

    if (IsKeyDown(KEY_D))
    {
        movement.x += 1.0f;
    }

    isMoving =
        Vector3Length(movement) > 0.0f;

    isRunning =
        isMoving &&
        IsKeyDown(KEY_LEFT_SHIFT);

    if (!isDashing && isMoving)
    {
        movement =
            Vector3Normalize(movement);

        facingDirection =
            movement;
    }

    if (
        IsKeyPressed(KEY_SPACE) &&
        !isDashing &&
        dashCooldownTimer <= 0.0f
        )
    {
        isDashing = true;
        dashTimer = dashDuration;
        dashCooldownTimer = dashCooldown;
    }

    if (isDashing)
    {
        position.x +=
            facingDirection.x *
            dashSpeed *
            deltaTime;

        position.z +=
            facingDirection.z *
            dashSpeed *
            deltaTime;

        dashTimer -= deltaTime;

        if (dashTimer <= 0.0f)
        {
            isDashing = false;
            dashTimer = 0.0f;
        }
    }
    else if (isMoving)
    {
        const float currentSpeed =
            isRunning
            ? runSpeed
            : walkSpeed;

        position.x +=
            movement.x *
            currentSpeed *
            deltaTime;

        position.z +=
            movement.z *
            currentSpeed *
            deltaTime;
    }

    effectTime += deltaTime;

    const float mapLimit = 23.5f;

    position.x =
        Clamp(
            position.x,
            -mapLimit,
            mapLimit
        );

    position.z =
        Clamp(
            position.z,
            -mapLimit,
            mapLimit
        );

    UpdateModelAnimation(deltaTime);
}

void Player::UpdateModelAnimation(float deltaTime)
{
    CharacterAssets& assets =
        GetCharacterAssets();

    if (
        !assets.modelLoaded ||
        assets.animations == nullptr ||
        assets.animationCount <= 0
        )
    {
        return;
    }

    int desiredAnimation =
        assets.idleAnimation;

    if (weapon.IsAttacking())
    {
        desiredAnimation =
            assets.attackAnimation;
    }
    else if (isDashing)
    {
        desiredAnimation =
            assets.rollAnimation;
    }
    else if (isRunning)
    {
        desiredAnimation =
            assets.runAnimation;
    }
    else if (isMoving)
    {
        desiredAnimation =
            assets.walkAnimation;
    }

    if (
        desiredAnimation < 0 ||
        desiredAnimation >= assets.animationCount ||
        desiredAnimation >=
        static_cast<int>(
            assets.animationUsable.size()
            ) ||
        assets.animationUsable[desiredAnimation] == 0
        )
    {
        return;
    }

    if (currentModelAnimation != desiredAnimation)
    {
        currentModelAnimation =
            desiredAnimation;

        modelAnimationFrame = 0.0f;
    }

    const ModelAnimation& animation =
        assets.animations[
            currentModelAnimation
        ];

#if defined(RAYLIB_VERSION_MAJOR) && (RAYLIB_VERSION_MAJOR >= 6)
    const int animationFrameCount =
        animation.keyframeCount;
#else
    const int animationFrameCount =
        animation.frameCount;

    if (animation.framePoses == nullptr)
    {
        return;
    }
#endif

    if (animationFrameCount <= 0)
    {
        return;
    }

    int frameIndex = 0;

    if (
        weapon.IsAttacking() &&
        currentModelAnimation ==
        assets.attackAnimation
        )
    {
        frameIndex =
            static_cast<int>(
                Clamp(
                    weapon.GetAttackProgress(),
                    0.0f,
                    1.0f
                ) *
                static_cast<float>(
                    animationFrameCount - 1
                    )
                );
    }
    else
    {
        float framesPerSecond = 24.0f;

        if (currentModelAnimation == assets.walkAnimation)
        {
            framesPerSecond = 28.0f;
        }
        else if (
            currentModelAnimation ==
            assets.runAnimation
            )
        {
            framesPerSecond = 38.0f;
        }
        else if (
            currentModelAnimation ==
            assets.rollAnimation
            )
        {
            framesPerSecond = 36.0f;
        }

        modelAnimationFrame +=
            deltaTime *
            framesPerSecond;

        frameIndex =
            static_cast<int>(
                modelAnimationFrame
                ) %
            animationFrameCount;
    }

#if !defined(RAYLIB_VERSION_MAJOR) || (RAYLIB_VERSION_MAJOR < 6)
    if (
        frameIndex < 0 ||
        frameIndex >= animationFrameCount ||
        animation.framePoses[frameIndex] == nullptr
        )
    {
        return;
    }
#endif

#if defined(RAYLIB_VERSION_MAJOR) && (RAYLIB_VERSION_MAJOR >= 6)
    ::UpdateModelAnimation(
        assets.model,
        animation,
        frameIndex
    );
#else
    if (assets.safeAnimationMeshes.empty())
    {
        return;
    }

    // raylib 5.5'in UpdateModelAnimation() fonksiyonu modeldeki
    // butun mesh'leri dolasir. Gecici model yalnizca guvenli,
    // iskeletli mesh'leri icerir. Vertex ve VBO verileri asil
    // modelle ortak oldugu icin sonuc DrawModelEx() ile cizilir.
    Model safeAnimationModel =
        assets.model;

    safeAnimationModel.meshCount =
        static_cast<int>(
            assets.safeAnimationMeshes.size()
            );

    safeAnimationModel.meshes =
        assets.safeAnimationMeshes.data();

    ::UpdateModelAnimation(
        safeAnimationModel,
        animation,
        frameIndex
    );
#endif
}

void Player::Draw() const
{
    const Vector3 forward =
        Vector3Normalize(
            facingDirection
        );

    const Vector3 groundPosition{
        position.x,
        position.y - 0.99f,
        position.z
    };

    // Yumuşak karakter gölgesi.
    DrawCylinder(
        groundPosition,
        0.72f,
        0.58f,
        0.025f,
        32,
        Fade(BLACK, 0.28f)
    );

    // Dash sırasında arkada kalan Aether izi.
    if (isDashing)
    {
        for (int trailIndex = 1; trailIndex <= 6; ++trailIndex)
        {
            const float distance =
                static_cast<float>(
                    trailIndex
                    ) *
                0.34f;

            const Vector3 trailPosition =
                AddScaled(
                    Vector3{
                        position.x,
                        position.y + 0.05f,
                        position.z
                    },
                    forward,
                    -distance
                );

            const float radius =
                0.34f -
                static_cast<float>(
                    trailIndex
                    ) *
                0.035f;

            const float alpha =
                0.25f -
                static_cast<float>(
                    trailIndex
                    ) *
                0.025f;

            DrawSphere(
                trailPosition,
                radius,
                Fade(
                    Color{ 80, 220, 255, 255 },
                    alpha
                )
            );
        }
    }

    CharacterAssets& assets =
        GetCharacterAssets();

    if (assets.modelLoaded)
    {
        float yaw =
            std::atan2(
                forward.x,
                forward.z
            ) *
            RAD2DEG;

        float attackLunge = 0.0f;

        if (weapon.IsAttacking())
        {
            const float attackProgress =
                Clamp(
                    weapon.GetAttackProgress(),
                    0.0f,
                    1.0f
                );

            // Saldiri sirasinda yalnizca model animasyonu ve kisa
            // ileri hamle kullanilir. Karakterin tum govdesi artik
            // saga-sola dondurulmaz.
            attackLunge =
                std::sin(
                    attackProgress *
                    PI
                ) *
                0.24f;
        }

        const Vector3 modelPosition{
            position.x,
            position.y -
                1.0f +
                assets.groundOffset,
            position.z
        };

        const Vector3 lungedModelPosition =
            AddScaled(
                modelPosition,
                forward,
                attackLunge
            );

        DrawModelEx(
            assets.model,
            lungedModelPosition,
            Vector3{ 0.0f, 1.0f, 0.0f },
            yaw,
            Vector3{
                assets.scale,
                assets.scale,
                assets.scale
            },
            isDashing
            ? Color{ 190, 245, 255, 255 }
            : WHITE
        );
    }
    else
    {
        DrawFallbackCharacter(
            position,
            forward
        );
    }

    // Modelin göğsündeki Aether parıltısı.
    const float pulse =
        0.5f +
        0.5f *
        std::sin(
            effectTime *
            4.0f
        );

    const Vector3 chestGlow =
        AddScaled(
            Vector3{
                position.x,
                position.y + 0.25f,
                position.z
            },
            forward,
            0.34f
        );

    DrawSphere(
        chestGlow,
        0.055f +
        pulse *
        0.025f,
        Color{ 100, 232, 255, 220 }
    );

    if (weapon.IsAttacking())
    {
        const float attackProgress =
            Clamp(
                weapon.GetAttackProgress(),
                0.0f,
                1.0f
            );

        const float attackPower =
            std::sin(
                attackProgress *
                PI
            );

        const int segmentCount = 14;

        for (
            int segment = 0;
            segment < segmentCount;
            ++segment
            )
        {
            const float segmentStart =
                static_cast<float>(
                    segment
                    ) /
                static_cast<float>(
                    segmentCount
                    );

            const float segmentEnd =
                static_cast<float>(
                    segment + 1
                    ) /
                static_cast<float>(
                    segmentCount
                    );

            if (
                segmentStart >
                attackProgress + 0.18f
                )
            {
                continue;
            }

            const float startAngle =
                -1.55f +
                segmentStart *
                2.70f;

            const float endAngle =
                -1.55f +
                segmentEnd *
                2.70f;

            const Vector3 startDirection =
                RotateOnGround(
                    forward,
                    startAngle
                );

            const Vector3 endDirection =
                RotateOnGround(
                    forward,
                    endAngle
                );

            Vector3 startPoint =
                AddScaled(
                    position,
                    startDirection,
                    1.48f
                );

            Vector3 endPoint =
                AddScaled(
                    position,
                    endDirection,
                    1.48f
                );

            startPoint.y +=
                0.50f +
                segmentStart *
                0.55f;

            endPoint.y +=
                0.50f +
                segmentEnd *
                0.55f;

            const float visibility =
                1.0f -
                std::fabs(
                    segmentEnd -
                    attackProgress
                );

            DrawCylinderEx(
                startPoint,
                endPoint,
                0.10f,
                0.035f,
                8,
                Fade(
                    Color{ 115, 230, 255, 255 },
                    Clamp(
                        visibility *
                        attackPower *
                        0.80f,
                        0.0f,
                        0.80f
                    )
                )
            );
        }
    }
}

Vector3 Player::GetPosition() const
{
    return position;
}

Vector3 Player::GetFacingDirection() const
{
    return facingDirection;
}

void Player::SetPosition(Vector3 newPosition)
{
    position = newPosition;
}

bool Player::IsAttacking() const
{
    return weapon.IsAttacking();
}

float Player::GetAttackProgress() const
{
    return weapon.GetAttackProgress();
}

void Player::TakeDamage(int damage)
{
    if (damage <= 0)
    {
        return;
    }

    const float damageMultiplier =
        1.0f - armor;

    int finalDamage =
        static_cast<int>(
            std::round(
                static_cast<float>(
                    damage
                    ) *
                damageMultiplier
            )
            );

    if (finalDamage < 1)
    {
        finalDamage = 1;
    }

    health -= finalDamage;

    if (health < 0)
    {
        health = 0;
    }
}

int Player::GetHealth() const
{
    return health;
}

int Player::GetMaxHealth() const
{
    return maxHealth;
}

void Player::Heal(int amount)
{
    if (amount <= 0)
    {
        return;
    }

    health += amount;

    if (health > maxHealth)
    {
        health = maxHealth;
    }
}

bool Player::IsAlive() const
{
    return health > 0;
}

void Player::IncreaseMaxHealth(int amount)
{
    if (amount <= 0)
    {
        return;
    }

    maxHealth += amount;
    health += amount;
}

void Player::IncreaseArmor(float amount)
{
    if (amount <= 0.0f)
    {
        return;
    }

    armor += amount;

    if (armor > 0.50f)
    {
        armor = 0.50f;
    }
}

void Player::IncreaseMovementSpeed(
    float walkAmount,
    float runAmount
)
{
    if (walkAmount > 0.0f)
    {
        walkSpeed += walkAmount;
    }

    if (runAmount > 0.0f)
    {
        runSpeed += runAmount;
    }
}

void Player::ReduceDashCooldown(float amount)
{
    if (amount <= 0.0f)
    {
        return;
    }

    dashCooldown -= amount;

    if (dashCooldown < 0.20f)
    {
        dashCooldown = 0.20f;
    }
}

float Player::GetArmor() const
{
    return armor;
}

float Player::GetWalkSpeed() const
{
    return walkSpeed;
}

float Player::GetRunSpeed() const
{
    return runSpeed;
}

float Player::GetDashCooldown() const
{
    return dashCooldown;
}
