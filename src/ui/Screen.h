#pragma once

#include <cstdint>
#include "button/Button.h"

// Forward declarations
class Adafruit_SSD1306;
class UIComponent;

/**
 * Classe base para telas do framework.
 *
 * Cada tela herda de Screen e pode compor UIComponents como filhos.
 * O render() base itera sobre os filhos na ordem de adição e chama
 * render() de cada um. Subclasses podem sobrescrever render() para
 * adicionar lógica customizada.
 *
 * O ciclo de vida (onMount/onUnmount) permite inicializar e liberar
 * recursos ao entrar/sair da tela. O dirty flag controla quando o
 * RenderLoop deve redesenhar a tela.
 *
 * Screen NÃO herda de UIComponent (composição sobre herança).
 */
class Screen {
public:
    /// Número máximo de componentes filhos por Screen.
    static constexpr uint8_t MAX_CHILDREN = 16;

    virtual ~Screen() = default;

    /// Chamado pelo Router ao entrar na tela (antes do primeiro render).
    virtual void onMount() {}

    /// Chamado pelo Router ao sair da tela.
    virtual void onUnmount() {}

    /// Desenha a tela no buffer do display.
    /// A implementação base itera sobre os filhos na ordem de adição
    /// e chama render() de cada um. Subclasses podem sobrescrever.
    virtual void render(Adafruit_SSD1306& display);

    /// Processa um evento de botão encaminhado pelo Router.
    virtual void handleInput(ButtonEvent event) {}

    /// Adiciona um componente filho à lista de renderização.
    /// Retorna true se adicionado com sucesso, false se a lista estiver cheia
    /// ou o ponteiro for nullptr.
    bool addChild(UIComponent* child);

    /// Retorna o número de filhos registrados.
    uint8_t getChildCount() const;

    /// Marca a tela como suja — o próximo update() irá redesenhá-la.
    void markDirty();

    /// Retorna true se a tela precisa ser redesenhada.
    bool isDirty() const;

    /// Limpa o dirty flag após o redesenho.
    void clearDirty();

private:
    bool _dirty = true; // Inicia como true para garantir o primeiro render
    UIComponent* _children[MAX_CHILDREN] = {};
    uint8_t _childCount = 0;
};
