# OOP Quiz Studio — C++ Mini Project

A small native Windows GUI quiz app for an Object-Oriented Programming (OOP) course. It uses only standard C++17 and the Windows API; no third-party GUI framework is required.

## Features

- Simple graphical interface with radio-button answers.
- Multiple-choice and True/False questions.
- Immediate answer feedback and explanations.
- Randomized question order and a final score/review screen.
- Reads editable questions from `questions.txt`.
- Appends each completed result to `results.txt`.
- Uses built-in sample questions if the question file is missing or invalid.
- Handles missing/malformed question files and result-saving failures without crashing.

## How it meets the course outcomes

| Course outcome | How the app demonstrates it |
|---|---|
| **CO1 — OOP concepts** | Classes encapsulate question and scoring data; the GUI uses objects and methods to manage quiz state. |
| **CO2 — Inheritance and reusability** | `MultipleChoiceQuestion` and `TrueFalseQuestion` inherit shared data and behaviour from the reusable `Question` base class. |
| **CO3 — Polymorphism** | `Question` declares virtual functions. The app stores derived questions using `std::unique_ptr<Question>` and calls overridden virtual methods through the base type. |
| **CO4 — Exception and file handling** | `std::ifstream` loads the question bank; `std::ofstream` saves results. File errors and malformed input are handled with exceptions and fallback questions. |
| **CO5 — Templates and STL** | `ScoreKeeper<T>` and `clampValue<T>` demonstrate templates. `std::vector`, `std::string`, `std::unique_ptr`, `std::shuffle`, and other STL utilities manage data. |

## Run on Windows

### Option A: MinGW-w64 / g++

1. Keep `oop_quiz_app.cpp` and `questions.txt` in the same folder.
2. Open Command Prompt or a terminal in that folder.
3. Compile:

   ```bash
   g++ -std=c++17 -municode -mwindows oop_quiz_app.cpp -o OOPQuizStudio.exe
   ```

4. Run `OOPQuizStudio.exe` from that same folder so it can find `questions.txt`.

### Option B: Visual Studio

1. Create a **Windows Desktop Application** or an empty C++ project.
2. Add `oop_quiz_app.cpp` to the project.
3. Set the language standard to **C++17** or newer.
4. Build and run. Keep `questions.txt` in the working directory configured in the project’s Debugging settings.

## Editing the question bank

Open `questions.txt`. Each non-comment line must contain exactly 8 fields separated by `|`.

Multiple choice:

```text
MCQ|Question text|Option A|Option B|Option C|Option D|1|Explanation for the answer.
```

The answer index is **zero-based**: `0` means option A, `1` means B, `2` means C, and `3` means D.

True/False:

```text
TF|Statement to judge|True|False|||0|Explanation for the answer.
```

For True/False, use answer index `0` for True and `1` for False. Keep the two empty option fields before the answer index.

## Files

- `oop_quiz_app.cpp` — complete C++ GUI application and OOP implementation.
- `questions.txt` — editable question bank.
- `results.txt` — created automatically when a quiz is completed.

## Notes for a project demonstration

In the presentation, show the inheritance hierarchy (`Question` → `MultipleChoiceQuestion` / `TrueFalseQuestion`), point out the overridden virtual function and base-class storage, then demonstrate loading questions and saving a result. This makes the CO2, CO3, and CO4 evidence easy to explain.
