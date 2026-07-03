# Proj_Elet.Pot_MPPT
Algoritmo P&amp;O para controle MPPT - Projeto Eletrônica de Potencia 01/2026
## Algoritmo MPPT: Perturba e Observa (P&O)

O principal mecanismo para o controle de energia deste projeto é o algoritmo **Perturba e Observa (P&O)**. Ele é uma técnica de Rastreamento do Ponto de Máxima Potência (MPPT) amplamente utilizada em sistemas fotovoltaicos devido à sua simplicidade de implementação e alta eficiência.

O objetivo principal do algoritmo é ajustar continuamente o ciclo de trabalho (*duty cycle*) do conversor Buck para fazer com que o painel solar opere sempre no seu "joelho" de máxima potência ($V_{mp} \approx 19.0\text{V}$), adaptando-se instantaneamente a mudanças na irradiação solar e temperatura.

### Como Funciona a Lógica?

O algoritmo funciona em um ciclo contínuo de três etapas: **Medir, Comparar e Ajustar**.

1. **Perturbação:** O Arduino altera levemente o sinal PWM (`valor_pwm`) que controla o chaveamento do MOSFET. Isso altera a impedância aparente do circuito e, consequentemente, a tensão do painel solar.
2. **Observação:** O sensor INA219 mede a nova tensão e a nova corrente, calculando a potência atual ($P = V \times I$).
3. **Comparação:** O algoritmo compara a potência atual com a potência registrada no ciclo anterior ($\Delta P$).

A partir dessa comparação, a lógica toma a decisão do próximo passo baseado no gráfico abaixo:

*   **Se a potência aumentou ($\Delta P > 0$):** Significa que a perturbação moveu o sistema na direção certa (em direção ao pico da curva de potência). Portanto, o algoritmo **mantém a mesma direção** do passo (continua aumentando ou diminuindo o PWM).
*   **Se a potência diminuiu ($\Delta P < 0$):** Significa que o sistema se afastou do ponto ideal. O algoritmo então **inverte a direção** do passo para voltar ao caminho correto.

### Fluxo de Decisão

A tabela verdade que dita o comportamento do código a cada ciclo de 50ms é:

| Mudança na Potência ($\Delta P$) | Direção do Passo Anterior | Ação no PWM |
| :--- | :--- | :--- |
| Aumentou ($+$) | Subindo ($+$) | **Aumentar** ($+$) |
| Aumentou ($+$) | Descendo ($-$) | **Diminuir** ($-$) |
| Diminuiu ($-$) | Subindo ($+$) | **Diminuir** ($-$) |
| Diminuiu ($-$) | Descendo ($-$) | **Aumentar** ($+$) |

### Adaptações de Segurança Implementadas

Embora o P&O tradicional busque sempre o limite máximo do painel (30W), este projeto implementa uma **trava de segurança por software** essencial para a preservação do hardware:

*   **Limitador de Potência (11W):** Como a bomba d'água utilizada possui especificação nominal de 10W, o código monitora a potência de entrada. Caso ela ultrapasse 11W, o algoritmo interrompe a busca pelo ponto máximo e reduz o PWM ativamente para proteger o motor contra sobrecarga, forçando o painel a operar fora do joelho sob sol pleno.
*   **Estabilização Elétrica:** O intervalo de amostragem foi fixado em 50ms (`delay(50)`), tempo ideal para que o indutor e os capacitores do conversor Buck estabilizem a tensão antes da próxima tomada de decisão do Arduino, evitando oscilações erráticas.

```mermaid
graph TD
    A([Início do Loop]) --> B[Mede V e I no INA219]
    B --> C[Calcula Potência Atual pot_in = tensao * corrente]
    C --> D{pot_in > 11.0W?}
    
    %% Bloco de Segurança
    D -- Sim --> E[valor_pwm = valor_pwm - 5]
    E --> F[Aplica Constrain 10 a 245]
    F --> G[analogWrite valor_pwm]
    G --> H[potAntes = pot_in]
    H --> I[Aguarda 50ms]
    I --> J[return: Reinicia o Loop]
    J --> A

    %% Lógica P&O Normal
    D -- Não --> K[Calcula delta_pot = pot_in - potAntes]
    K --> L{delta_pot < 0?}
    L -- Sim --> M[Inverte o sinal do passo] --> N[valor_pwm = valor_pwm + passo]
    L -- Não --> N
    N --> O[potAntes = pot_in]
    O --> P[Aplica Constrain 10 a 245]
    P --> Q[analogWrite valor_pwm]
    Q --> R[Aguarda 50ms]
    R --> A
