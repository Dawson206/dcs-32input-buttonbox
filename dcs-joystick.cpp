#include <SPI.h>
#include <Joystick.h>

// These are macros.  Anytime we see BUTTON_COUNT, replace it with 32, for example.
// Change these values if the pins, buttons, or amount of 74HC165 changes.
#define BUTTON_SAMPLE_PIN 7
#define BUTTON_COUNT 32
#define BUTTON_SHIFT_REGISTER_COUNT 4

// Create the Joystick object to act like a joystick.
Joystick_ Joystick;

// Initialize button state variables that can be read and written to from anywhere.
uint32_t button_states = 0;
uint32_t last_button_states;

void sample_button_states() {
  digitalWrite(BUTTON_SAMPLE_PIN, LOW);
  digitalWrite(BUTTON_SAMPLE_PIN, HIGH);
}

void read_button_states() {
  // Save the last states before reading new states.
  last_button_states = button_states;
  // Clear the states in preparation for reading new states.
  button_states = 0;

  // For each shift register (74HC165)...
  for (int shift_register = 0; shift_register < BUTTON_SHIFT_REGISTER_COUNT; shift_register++) {
    // 1. Read the shift register values: SPI.transfer(0).
    //    The 0 means we're also writing 0 while reading data.
    //    We never write anything to these registers, so we just use 0 to keep it happy.
    //    All buttons held down would be 0b11111111.
    //    All buttons released would be 0b00000000.
    // 2. Shift the values into its own spot: << shift_register * 8
    //    We shift in multiples of 8 to read 8 bits (one byte) at a time.
    //    i.e. with leading zeroes: 0b0000000011111111 << 0 remains as 0b0000000011111111
    //    i.e. with leading zeroes: 0b0000000011111111 << 8 becomes 0b1111111100000000
    // 3. Add the value to the combined button states variable: button_states +=
    //    i.e. 0b0000000011111111 + 0b1111111100000000 == 0b1111111111111111
    button_states += (uint32_t)(SPI.transfer(0)) << shift_register * 8;
  }
}

void write_joystick() {
  // For each joystick button...
  for (int button = 0; button < BUTTON_COUNT; button++) {
    // Create a button mask used to read only one bit from the button states.
    // 1 << 0 is 0b00000001, 1 << 1 is 0b00000010, 1 << 2 is 0b00000100, etc.
    uint32_t button_mask = (uint32_t)(1) << button;

    // Use the mask to do "bitwise and" math on the button states.

    // if the mask is...      00000000 00000000 00000000 00000001
    // and the buttons are... 00000100 00001000 00000100 00000101
    // then we get...         00000000 00000000 00000000 00000001 <- there's a value

    // if the mask is...      00000000 00000000 00000000 00000010
    // and the buttons are... 00000100 00001000 00000100 00000101
    // then we get...         00000000 00000000 00000000 00000000 <- there isn't a value

    // if the mask is...      00000000 00000000 00000000 00000100
    // and the buttons are... 00000100 00001000 00000100 00000101
    // then we get...         00000000 00000000 00000000 00000100 <- there's a value

    // With that value, store it as a bool, or boolean.
    // Booleans only store 1 or 0, or true or false.
    // Turning a non-boolean value into a boolean value is called casting.
    // int 0 (0b00000000) casts to bool 0 (false).
    // int 1 (0b00000001) casts to bool 1 (true).
    // int 2 (0b00000010) casts to bool 1 (true).
    // int 4 (0b00000100) casts to bool 1 (true).
    // If a button is pushed, it's true, if it isn't, it's false.
    bool button_state = button_states & button_mask;

    // Do the same thing with the last button states, too.
    bool last_button_state = last_button_states & button_mask;

    // If the new state is different than the old state...
    if (button_state != last_button_state) {
      // Set the joystick button in this loop to the button state.
      Joystick.setButton(button, button_state);
    }
  }
}

void setup() {
  // Make this pin an output pin to tell the 74HC165s to sample data.
  pinMode(BUTTON_SAMPLE_PIN, OUTPUT);

  // Setup SPI.  This is used to read data from the 74HC165s.
  SPI.begin();

  // Setup the joystick.
  Joystick.begin();
}

void loop() {
  sample_button_states();
  read_button_states();
  write_joystick();
}
