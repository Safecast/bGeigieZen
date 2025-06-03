// ULP program: counts rising edges on GPIO32
  .text
  .global ulp_prog
ulp_prog:
  // Label 0: Start of loop
  M_LABEL(0)

  // R3 = SLOW_LAST_STATE index
  I_MOVI(R3, SLOW_LAST_STATE)
  // R0 = RTC_SLOW_MEM[SLOW_LAST_STATE] (last pin state)
  I_LD(R0, R3, 0)

  // Read GPIO32 (RTC_GPIO12, channel 12)
  I_RD_REG(RTC_GPIO_IN_REG, 12, 12) // R1 = (GPIO32 state)
  I_MOVI(R2, 0)
  I_ORI(R2, R1, 0) // R2 = current pin state (using ORI for effective move)

  // --- Rising Edge Detection: Check if last state was 0 and current state is 1 ---

  // If R0 (last state) is NOT 0, jump to label 2 (skip pulse increment).
  // If R0 IS 0, fall through to check current state.
  M_BNE(M_LABEL(2), 0)

  // Check if R0 (last state) is 0.
  // If R0 IS 0, continue to check current state (fall through).
  // If R0 is NOT 0, branch to label 2 (skip pulse increment).
  M_BEQ(M_LABEL(10), 0) // If R0 == 0, branch to label 10 (check_current_state)
  M_B(M_LABEL(2))     // If R0 != 0, branch to label 2 (skip increment)

  M_LABEL(10) // Label to check current state
  // If R0 was 0, now check if R2 (current state) is 1.
  // If R2 IS 1, continue to increment count (fall through to label 1).
  // If R2 is NOT 1, branch to label 2 (skip pulse increment).
  I_MOVI(R1, 1) // Load value 1 into R1 for comparison
  M_BNE(M_LABEL(2), R1) // If R2 != 1, branch to label 2

  // Removed the explicit M_BNE check here and updated the logic below to rely on M_BEQ

  M_LABEL(10) // Label to check current state
  // Check if R2 (current state) is 1.
  // If R2 IS 1, fall through to increment count (label 1).
  // If R2 is NOT 1, branch to label 2 (skip pulse increment).
  I_MOVI(R1, 1) // Load value 1 into R1 for comparison
  M_BNE(M_LABEL(2), R1) // If R2 != 1, branch to label 2

  // If both checks passed (last state was 0 and current state is 1), it's a rising edge

  // Label 1: rising edge detected - increment pulse count
  M_LABEL(1)
  I_MOVI(R3, SLOW_PULSE_COUNT)
  I_LD(R0, R3, 0) // R0 = pulse count
  I_ADDI(R0, R0, 1) // R0++
  I_ST(R0, R3, 0)   // Store back

  // Label 2: store current pin state as last state
  M_LABEL(2)
  I_MOVI(R3, SLOW_LAST_STATE)
  I_ST(R2, R3, 0) // Store R2 (current state) as the new last state

  // Delay (adjust as needed, e.g. 10ms)
  I_DELAY(10000)

  // Loop back to the start (label 0)
  M_B(M_LABEL(0)) // Unconditional branch to label 0 