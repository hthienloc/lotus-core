import sys
import os

# Add the parent directory to sys.path so we can import the lotus_engine module
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

from python.lotus_engine import LotusEngine, Method, ToneStyle, FreeW, LogLevel

def log_handler(level: int, message: str):
    prefix = ["[DEBUG]", "[INFO]", "[ERROR]"]
    print(f"Log: {prefix[level] if level < 3 else '[UNKNOWN]'} {message}")

def process_and_print(engine: LotusEngine, text: str):
    """Processes a string of characters one by one, keeping track of the current string state."""
    print(f"\nTyping: '{text}'")
    current_str = []
    
    for char in text:
        # Check if it's an uppercase character (simplified shift logic)
        is_upper = char.isupper()
        
        action, backspace, output_str = engine.process_key(char, shift=is_upper)
        
        # Apply backspaces
        if backspace > 0:
            current_str = current_str[:-backspace]
            
        # Apply output string
        if action == 1 and output_str: # Transformation
            current_str.extend(list(output_str))
        else: # Passthrough or action 0 with output
            # if passthrough, we just add the character itself
            if output_str:
                 current_str.extend(list(output_str))
            elif action == 0:
                 current_str.append(char)
        
        print(f"  Key: '{char}' -> Action: {action}, Backspace: {backspace}, Output: '{output_str}' => State: {''.join(current_str)}")
        
    print(f"Result: '{''.join(current_str)}'")
    return ''.join(current_str)

def main():
    print("Lotus Engine Python Demo")
    print("------------------------")
    
    # 1. Setup Logging
    LotusEngine.set_log_callback(log_handler)
    
    # 2. Create Engine
    print("\nInitializing Engine...")
    engine = LotusEngine()
    
    # 3. Configure Engine
    engine.set_method(Method.TELEX)
    engine.set_tone_style(ToneStyle.NEW)
    engine.set_free_w(FreeW.ALWAYS)
    engine.set_auto_restore(True)
    print("Configuration: TELEX, NEW tone style, Free W ALWAYS, Auto Restore ON")
    
    # 4. Add a Shortcut
    engine.add_shortcut("vn", "Việt Nam")
    print("Added shortcut: 'vn' -> 'Việt Nam'")
    
    # 5. Process Some Text
    print("\n--- Testing Basic Telex ---")
    process_and_print(engine, "xin chao")
    engine.reset() # Important: Reset engine between words if there's no space
    
    process_and_print(engine, "xin chao2")
    engine.reset()
    
    print("\n--- Testing Tones and Markers ---")
    process_and_print(engine, "tieengs Vieetj")
    engine.reset()
    
    print("\n--- Testing Shortcuts ---")
    process_and_print(engine, "vn ")
    engine.reset()
    
    print("\n--- Testing English Restoration ---")
    process_and_print(engine, "expect")
    engine.reset()
    
if __name__ == "__main__":
    main()
