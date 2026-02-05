Import("env")
import platform
import re

# Fix for macOS shell syntax error with parentheses in build flags
if platform.system() == "Darwin":
    # Get current CPPDEFINES
    cppdefines = env.get("CPPDEFINES", [])
    new_defines = []
    
    for define in cppdefines:
        if isinstance(define, tuple):
            name, value = define
            # Replace problematic ARDUINO_BOARD with safe version
            if name == "ARDUINO_BOARD":
                new_defines.append(("ARDUINO_BOARD", '\\"ESP32-S3-DevKitC-1\\"'))
            else:
                new_defines.append(define)
        elif isinstance(define, str):
            new_defines.append(define)
        else:
            new_defines.append(define)
    
    env.Replace(CPPDEFINES=new_defines)
    
    # Also fix CCFLAGS and CXXFLAGS that might contain unescaped parens
    for flag_type in ["CCFLAGS", "CXXFLAGS", "CFLAGS"]:
        flags = env.get(flag_type, [])
        if flags:
            new_flags = []
            for flag in flags:
                if isinstance(flag, str) and "(" in flag:
                    # Escape for shell
                    new_flags.append(flag.replace("(", "\\(").replace(")", "\\)"))
                else:
                    new_flags.append(flag)
            env.Replace(**{flag_type: new_flags})
    
    print("macOS build fix applied: Sanitized defines with parentheses")
