#include "imgui_system.h"
#include "../characters/character.h"

#include <cmath>
#include <iostream>

bool ImGuiSystem::initialize(GLFWwindow *window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, false);
    ImGui_ImplOpenGL3_Init("#version 330");
    initialized = true; 
    return true;
}

void ImGuiSystem::beginFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();

    ImGui::NewFrame();
    ImGui::Begin("Engine Debug");

    ImGui::Text("Physics Engine");
    ImGui::Separator();
    ImGui::Checkbox("Demo Window", &showDemoWindow);
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::End();

    if (showDemoWindow)
        ImGui::ShowDemoWindow(&showDemoWindow);
    renderLightEditor();
    renderModelInspector();
}

void ImGuiSystem::renderLightEditor() {

    if (!m_lightManager) {
        std::cout << "[ImGuiSystem] Warning: LightManager not set, cannot render light editor.\n";
        return;
    }
    LightManager &lm = *m_lightManager;
    ImGui::SetNextWindowSize(ImVec2(380, 600), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_FirstUseEver);
    ImGui::Begin("Lights", nullptr, ImGuiWindowFlags_None);
    ImGui::Text("%d / %d lights", lm.count(), MAX_LIGHTS);
    ImGui::SameLine();
    bool addDisabled = (lm.count() >= MAX_LIGHTS);

    if (addDisabled)
        ImGui::BeginDisabled();

    if (ImGui::Button("+ Add"))
        ImGui::OpenPopup("##addlightmenu");

    if (ImGui::BeginPopup("##addlightmenu")) {
        auto doAdd = [&](LightType t, const char *name) {
            Light l;
            l.name = name;
            l.type = t;

            if (t == LightType::Directional) {
                l.intensity = 3.0f;
                l.color = glm::vec3(1.0f, 0.95f, 0.8f);
            } else if (t == LightType::Spot) {
                l.intensity = 10.0f;
                l.position = glm::vec3(0.0f, 5.0f, 0.0f);
            } else {
                l.intensity = 5.0f;
                l.position = glm::vec3(0.0f, 3.0f, 0.0f);
            }
            m_selectedLight = lm.addLight(l);
            lm.save();
            ImGui::CloseCurrentPopup();
        };

        if (ImGui::MenuItem("Point Light"))
            doAdd(LightType::Point, "Point Light");

        if (ImGui::MenuItem("Directional Light"))
            doAdd(LightType::Directional, "Directional Light");

        if (ImGui::MenuItem("Spot Light"))
            doAdd(LightType::Spot, "Spot Light");
        ImGui::EndPopup();
    }

    if (addDisabled)
        ImGui::EndDisabled();
    ImGui::SameLine();
    bool removeDisabled = (m_selectedLight < 0 || m_selectedLight >= lm.count());

    if (removeDisabled)
        ImGui::BeginDisabled();

    if (ImGui::Button("Remove")) {
        lm.removeLight(m_selectedLight);
        lm.save();
        m_selectedLight = std::min(m_selectedLight, lm.count() - 1);
    }

    if (removeDisabled)
        ImGui::EndDisabled();
    ImGui::SameLine();
    bool dupDisabled = removeDisabled || (lm.count() >= MAX_LIGHTS);

    if (dupDisabled)
        ImGui::BeginDisabled();

    if (ImGui::Button("Duplicate")) {
        lm.duplicateLight(m_selectedLight);
        m_selectedLight = m_selectedLight + 1;
        lm.save();
    }

    if (dupDisabled)
        ImGui::EndDisabled();
    ImGui::SameLine();

    if (ImGui::Button("Save"))
        lm.save();
    ImGui::Separator();
    ImGui::BeginChild("##lightlist", ImVec2(130, 0),
                      true, ImGuiWindowFlags_HorizontalScrollbar);
    for (int i = 0; i < lm.count(); ++i) {
        Light &l = lm.get(i);
        ImVec4 dotCol = l.enabled
                            ? ImVec4(l.color.r, l.color.g, l.color.b, 1.0f)
                            : ImVec4(0.4f, 0.4f, 0.4f, 1.0f);
        ImGui::PushStyleColor(ImGuiCol_Text, dotCol);
        const char *icon = (l.type == LightType::Point)         ? "[P]"
                           : (l.type == LightType::Directional) ? "[D]"
                                                                : "[S]";
        ImGui::TextUnformatted(icon);
        ImGui::PopStyleColor();
        ImGui::SameLine();
        bool selected = (i == m_selectedLight);
        char label[64];
        snprintf(label, sizeof(label), "%s##li%d", l.name.c_str(), i);

        if (ImGui::Selectable(label, selected))
            m_selectedLight = i;
    }
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("##lightinspector", ImVec2(0, 0), false);

    if (m_selectedLight >= 0 && m_selectedLight < lm.count()) {
        Light &l = lm.get(m_selectedLight);
        bool dirty = false;
        char nameBuf[128];
        snprintf(nameBuf, sizeof(nameBuf), "%s", l.name.c_str());
        ImGui::SetNextItemWidth(-1);

        if (ImGui::InputText("##lname", nameBuf, sizeof(nameBuf))) {
            l.name = nameBuf;
            dirty = true;
        }
        ImGui::Checkbox("Enabled", &l.enabled);
        ImGui::SameLine();
        const char *typeNames[] = {"Point", "Directional", "Spot"};
        int typeIdx = (int)l.type;
        ImGui::SetNextItemWidth(120);

        if (ImGui::Combo("Type##ltype", &typeIdx, typeNames, 3)) {
            l.type = (LightType)typeIdx;
            dirty = true;
        }
        ImGui::Separator();
        float col[3] = {l.color.r, l.color.g, l.color.b};

        if (ImGui::ColorEdit3("Color", col,
                              ImGuiColorEditFlags_Float |
                                  ImGuiColorEditFlags_HDR |
                                  ImGuiColorEditFlags_PickerHueBar)) {
            l.color = glm::vec3(col[0], col[1], col[2]);
            dirty = true;
        }
        float logIntensity = std::log10(std::max(l.intensity, 0.001f));

        if (ImGui::SliderFloat("Intensity (log)", &logIntensity, -2.0f, 5.0f)) {
            l.intensity = std::pow(10.0f, logIntensity);
            dirty = true;
        }
        ImGui::SameLine();

        if (ImGui::InputFloat("##intraw", &l.intensity, 0.1f, 1.0f, "%.3f",
                              ImGuiInputTextFlags_EnterReturnsTrue)) {
            l.intensity = std::max(0.0f, l.intensity);
            dirty = true;
        }
        ImGui::Separator();

        if (l.type != LightType::Directional) {
            float pos[3] = {l.position.x, l.position.y, l.position.z};

            if (ImGui::DragFloat3("Position", pos, 0.1f)) {
                l.position = glm::vec3(pos[0], pos[1], pos[2]);
                dirty = true;
            }
        }

        if (l.type != LightType::Point) {
            float dir[3] = {l.direction.x, l.direction.y, l.direction.z};

            if (ImGui::DragFloat3("Direction", dir, 0.01f, -1.0f, 1.0f)) {
                glm::vec3 d(dir[0], dir[1], dir[2]);
                float len = glm::length(d);
                l.direction = (len > 0.0001f) ? d / len : glm::vec3(0.0f, -1.0f, 0.0f);
                dirty = true;
            }

            if (l.type == LightType::Directional) {
                float elev = glm::degrees(std::asin(-l.direction.y));
                float azim = glm::degrees(std::atan2(l.direction.x, l.direction.z));
                bool angChanged = false;
                angChanged |= ImGui::SliderFloat("Elevation", &elev, -90.0f, 90.0f, "%.1f°");
                angChanged |= ImGui::SliderFloat("Azimuth", &azim, -180.0f, 180.0f, "%.1f°");

                if (angChanged) {
                    float re = glm::radians(elev);
                    float ra = glm::radians(azim);
                    l.direction.x = std::cos(re) * std::sin(ra);
                    l.direction.y = -std::sin(re);
                    l.direction.z = std::cos(re) * std::cos(ra);
                    l.direction = glm::normalize(l.direction);
                    dirty = true;
                }
            }
        }

        if (l.type != LightType::Directional) {
            ImGui::Separator();
            ImGui::Text("Attenuation");
            dirty |= ImGui::DragFloat("Constant", &l.constant, 0.001f, 0.0f, 5.0f, "%.4f");
            dirty |= ImGui::DragFloat("Linear", &l.linear, 0.001f, 0.0f, 2.0f, "%.4f");
            dirty |= ImGui::DragFloat("Quadratic", &l.quadratic, 0.001f, 0.0f, 2.0f, "%.4f");
            float reach = 0.0f;

            if (l.quadratic > 0.0001f) {
                float thresh = 256.0f;
                float sq = l.linear * l.linear - 4.0f * l.quadratic * (l.constant - thresh);

                if (sq >= 0)
                    reach = (-l.linear + std::sqrt(sq)) / (2.0f * l.quadratic);
            }

            if (reach > 0.0f)
                ImGui::TextDisabled("Effective range ≈ %.1f units", reach);
        }

        if (l.type == LightType::Spot) {
            ImGui::Separator();
            ImGui::Text("Spot Cone");
            dirty |= ImGui::SliderFloat("Inner°", &l.innerCutoff, 1.0f, 89.0f, "%.1f");
            dirty |= ImGui::SliderFloat("Outer°", &l.outerCutoff, 1.0f, 90.0f, "%.1f");

            if (l.innerCutoff > l.outerCutoff) {
                l.innerCutoff = l.outerCutoff;
                dirty = true;
            }
        }

        if (dirty)
            lm.save();
    } else {
        ImGui::TextDisabled("Select a light to edit it.");
    }
    ImGui::EndChild();
    ImGui::End();
}

void ImGuiSystem::renderModelInspector() {

    if (!m_modelLoader)
        return;
    const size_t modelCount = m_modelLoader->getModelCount();

    ImGui::SetNextWindowSize(ImVec2(420, 600), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(415, 20), ImGuiCond_FirstUseEver);
    ImGui::Begin("Scene / Materials", nullptr, ImGuiWindowFlags_None);
    ImGui::BeginChild("##modeltree", ImVec2(148, 0), true,
                      ImGuiWindowFlags_HorizontalScrollbar);

    for (int mi = 0; mi < (int)modelCount; ++mi) {
        auto &data = m_modelLoader->getModel(mi);
        Model *model = data.model.get();

        if (!model)
            continue;

        char modelLabel[64];
        snprintf(modelLabel, sizeof(modelLabel), "[M] Model %d##m%d", mi, mi);
        ImGuiTreeNodeFlags treeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;

        if (m_selectedModel == mi && m_selectedMesh < 0)
            treeFlags |= ImGuiTreeNodeFlags_Selected;
        bool modelOpen = ImGui::TreeNodeEx(modelLabel, treeFlags);

        if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
            m_selectedModel = mi;
            m_selectedMesh = -1;
        }

        if (modelOpen) {
            for (int si = 0; si < (int)model->getMeshCount(); ++si) {
                MeshInstance &inst = model->getMeshAt(si);

                if (inst.name.rfind("Collider_", 0) == 0)
                    continue;

                const char *displayName =
                    !inst.mesh.material.name.empty()
                        ? inst.mesh.material.name.c_str()
                        : inst.name.c_str();
                char meshLabel[128];
                snprintf(meshLabel, sizeof(meshLabel),
                         "%s##ms%d_%d", displayName, mi, si);
                bool selected = (m_selectedModel == mi && m_selectedMesh == si);

                if (ImGui::Selectable(meshLabel, selected,
                                      ImGuiSelectableFlags_SpanAllColumns)) {
                    m_selectedModel = mi;
                    m_selectedMesh = si;
                }
            }
            ImGui::TreePop();
        }
    }
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("##matinspector", ImVec2(0, 0), false);

    bool validSelection =
        m_selectedModel >= 0 && m_selectedModel < (int)modelCount &&
        m_selectedMesh >= 0;

    if (validSelection) {
        Model *model = m_modelLoader->getModel(m_selectedModel).model.get();

        if (model && m_selectedMesh < (int)model->getMeshCount()) {
            MeshInstance &inst = model->getMeshAt(m_selectedMesh);
            renderMaterialEditor(inst.mesh.material, inst.name.c_str());
        }
    } else {
        ImGui::TextDisabled("Select a mesh to inspect its material.");
    }
    ImGui::EndChild();
    ImGui::End();
}

void ImGuiSystem::renderMaterialEditor(PBRMaterial &mat, const char *meshNodeName) {

    ImGui::Text("Mesh node: %s", meshNodeName);

    if (!mat.name.empty() && mat.name != meshNodeName)
        ImGui::TextDisabled("Material: %s", mat.name.c_str());

    ImGui::Spacing();

    auto thumb = [](unsigned int id, const char *label) {

        if (id)
            ImGui::Image((ImTextureID)(intptr_t)id,
                         ImVec2(48.0f, 48.0f));
        else {
            ImGui::Dummy(ImVec2(48.0f, 48.0f));
            ImGui::SameLine();
            ImGui::TextDisabled("(no %s)", label);
        }
    };

    if (ImGui::CollapsingHeader("Albedo", ImGuiTreeNodeFlags_DefaultOpen)) {
        thumb(mat.albedoTexID, "albedo");

        ImGui::SameLine();
        ImGui::BeginGroup();
        ImGui::Checkbox("Use Map##alb", &mat.useAlbedoMap);
        ImGui::SetNextItemWidth(180.0f);

        float bc[3] = {mat.baseColor.r, mat.baseColor.g, mat.baseColor.b};

        if (ImGui::ColorEdit3("Tint##bc", bc))
            mat.baseColor = glm::vec3(bc[0], bc[1], bc[2]);
        ImGui::EndGroup();

    }

    if (ImGui::CollapsingHeader("Normal Map", ImGuiTreeNodeFlags_DefaultOpen)) {
        thumb(mat.normalTexID, "normal");
        ImGui::SameLine();

        if (mat.hasNormalMap)
            ImGui::Checkbox("Use Map##nm", &mat.useNormalMap);
        else
            ImGui::TextDisabled("(not loaded)");
    }

    if (ImGui::CollapsingHeader("Metallic / Roughness", ImGuiTreeNodeFlags_DefaultOpen)) {
        thumb(mat.metallicRoughnessTexID, "MR map");
        ImGui::SameLine();
        ImGui::BeginGroup();

        if (mat.hasMetallicRoughnessMap) {
            ImGui::Checkbox("Use Map##mr", &mat.useMetallicRoughnessMap);

            if (mat.useMetallicRoughnessMap)
                ImGui::TextDisabled("(values overridden by map)");
        } else {
            ImGui::TextDisabled("(no MR map — scalars active)");
        }
        ImGui::SetNextItemWidth(180.0f);
        ImGui::SliderFloat("Metallic##m", &mat.metallic, 0.0f, 1.0f);
        ImGui::SetNextItemWidth(180.0f);
        ImGui::SliderFloat("Roughness##r", &mat.roughness, 0.0f, 1.0f);
        ImGui::EndGroup();
    }

    if (ImGui::CollapsingHeader("Ambient Occlusion", ImGuiTreeNodeFlags_DefaultOpen)) {
        thumb(mat.aoTexID, "AO map");
        ImGui::SameLine();
        ImGui::BeginGroup();

        if (mat.hasAOMap)
            ImGui::Checkbox("Use Map##ao", &mat.useAOMap);
        else
            ImGui::TextDisabled("(no AO map)");
        ImGui::SetNextItemWidth(180.0f);
        ImGui::SliderFloat("AO Strength##aos", &mat.ao, 0.0f, 1.0f);
        ImGui::EndGroup();
    }

    if (ImGui::CollapsingHeader("Emissive", ImGuiTreeNodeFlags_DefaultOpen)) {
        thumb(mat.emissiveTexID, "emissive");
        ImGui::SameLine();
        ImGui::BeginGroup();

        if (mat.hasEmissiveMap) {
            ImGui::Checkbox("Use Map##em", &mat.useEmissiveMap);
            ImGui::SetNextItemWidth(180.0f);
            float ec[3] = {mat.emissive.r, mat.emissive.g, mat.emissive.b};

            if (ImGui::ColorEdit3("Tint##ec", ec,
                                  ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float))
                mat.emissive = glm::vec3(ec[0], ec[1], ec[2]);
        } else {

            ImGui::TextDisabled("(no map — colour below drives emission)");
            ImGui::SetNextItemWidth(180.0f);
            float ec[3] = {mat.emissive.r, mat.emissive.g, mat.emissive.b};

            if (ImGui::ColorEdit3("Color##ec", ec,
                                  ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float))
                mat.emissive = glm::vec3(ec[0], ec[1], ec[2]);
        }
        ImGui::SetNextItemWidth(180.0f);
        ImGui::SliderFloat("Intensity##ei", &mat.emissiveIntensity, 0.0f, 20.0f);
        ImGui::EndGroup();
    }
}

void ImGuiSystem::onLevelChanged() {
    m_selectedModel = -1;
    m_selectedMesh  = -1;
    m_selectedLight = -1;
}

void ImGuiSystem::render() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void ImGuiSystem::shutdown() {

    if (!initialized)
        return;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    initialized = false;
}

void ImGuiSystem::renderPlayerHUD(const Character *player) {

    if (!player)
        return;
    const float barW = 220.0f;
    const float barH = 18.0f;
    const float pad = 12.0f;
    const float textW = 110.0f;
    ImGuiIO &io = ImGui::GetIO();
    ImVec2 windowPos(pad, io.DisplaySize.y - barH - pad * 2.0f - 4.0f);
    ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(barW + textW + pad * 2.0f,
                                    barH + pad * 2.0f),
                             ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.55f);
    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoInputs |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoNav;
    ImGui::Begin("##PlayerHUD", nullptr, flags);
    const float pct = player->getHealth() / player->getMaxHealth();
    ImDrawList *dl = ImGui::GetWindowDrawList();
    const ImVec2 p = ImGui::GetCursorScreenPos();
    dl->AddRectFilled(p,
                      ImVec2(p.x + barW, p.y + barH),
                      IM_COL32(35, 35, 35, 220), 4.0f);
    const ImU32 fillCol = IM_COL32(
        (ImU8)((1.0f - pct) * 255),
        (ImU8)(pct * 220),
        25,
        255);
    dl->AddRectFilled(ImVec2(p.x + 2.0f, p.y + 2.0f),
                      ImVec2(p.x + 2.0f + (barW - 4.0f) * pct, p.y + barH - 2.0f),
                      fillCol, 2.0f);
    dl->AddRect(p,
                ImVec2(p.x + barW, p.y + barH),
                IM_COL32(200, 200, 200, 180), 4.0f, 0, 1.5f);
    ImGui::Dummy(ImVec2(barW, barH));
    ImGui::SameLine(0.0f, 8.0f);
    ImGui::Text("HP  %.0f / %.0f", player->getHealth(), player->getMaxHealth());
    ImGui::End();
}

bool ImGuiSystem::worldToScreen(const glm::vec3 &worldPos,
                                const glm::mat4 &viewProj,
                                unsigned int screenW, unsigned int screenH,
                                glm::vec2 &out) {
    glm::vec4 clip = viewProj * glm::vec4(worldPos, 1.0f);

    if (clip.w <= 0.0f)
        return false;
    glm::vec3 ndc = glm::vec3(clip) / clip.w;

    if (ndc.z > 1.0f)
        return false;
    out.x = (ndc.x * 0.5f + 0.5f) * (float)screenW;
    out.y = (1.0f - (ndc.y * 0.5f + 0.5f)) * (float)screenH;
    return true;
}

void ImGuiSystem::renderBotHealthBars(
    const std::vector<std::shared_ptr<Character>> &bots,
    const glm::mat4 &view,
    const glm::mat4 &projection,
    unsigned int screenWidth,
    unsigned int screenHeight) {

    if (bots.empty())
        return;
    glm::mat4 viewProj = projection * view;
    ImDrawList *dl = ImGui::GetBackgroundDrawList();
    const float barW = 60.0f;
    const float barH = 7.0f;
    const float yOff = -20.0f;
    for (const auto &bot : bots) {

        if (!bot || bot->isDead())
            continue;

        glm::vec3 headPos = bot->getPosition() + glm::vec3(0.0f, 2.0f, 0.0f);
        glm::vec2 screen;

        if (!worldToScreen(headPos, viewProj, screenWidth, screenHeight, screen))
            continue;
        float pct = bot->getHealth() / bot->getMaxHealth();
        ImVec2 tl(screen.x - barW * 0.5f, screen.y + yOff);
        ImVec2 br(screen.x + barW * 0.5f, screen.y + yOff + barH);

        dl->AddRectFilled(tl, br, IM_COL32(30, 30, 30, 180), 3.0f);

        ImVec2 fillBr(tl.x + barW * pct, br.y);
        ImU32 fillCol = IM_COL32(
            (ImU8)((1.0f - pct) * 255),
            (ImU8)(pct * 220),
            25,
            220);
        dl->AddRectFilled(tl, fillBr, fillCol, 3.0f);

        dl->AddRect(tl, br, IM_COL32(200, 200, 200, 120), 3.0f);
    }
}