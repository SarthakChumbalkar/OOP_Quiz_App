# Mini Project Report: OOP Quiz Studio

## 1. Title
**OOP Quiz Studio — A C++ GUI Quiz Application**

## 2. Problem statement
Students need a simple way to practise Object-Oriented Programming topics and get immediate feedback. A quiz tool should present questions, check answers, explain correct answers, calculate a score, and preserve a record of completed attempts.

## 3. Objectives
- Build a simple graphical quiz application in C++.
- Apply classes, encapsulation, inheritance, and polymorphism.
- Handle question and result files safely.
- Use templates and Standard Template Library (STL) components.
- Provide instant feedback and a final score summary.

## 4. Software and hardware requirements
- Operating system: Windows 10 or Windows 11.
- Language: C++17.
- Compiler: MinGW-w64 g++ or Microsoft Visual Studio C++.
- GUI: Windows API (Win32); no third-party libraries are required.
- Hardware: any ordinary PC capable of running Windows and a C++ compiler.

## 5. Main modules
1. **Question model:** stores the prompt, options, correct answer, and explanation.
2. **Question types:** multiple-choice and true/false classes share the base Question class.
3. **Question bank:** stores the questions and randomizes their order.
4. **Quiz controller:** tracks the current question, selected answer, and score.
5. **File module:** loads `questions.txt` and appends completed results to `results.txt`.
6. **GUI module:** displays questions, radio-button options, feedback, and the results screen.

## 6. OOP implementation
- **Encapsulation:** question fields and score state are managed through classes and methods.
- **Inheritance:** `MultipleChoiceQuestion` and `TrueFalseQuestion` derive from `Question`.
- **Polymorphism:** a collection of base-class pointers holds different question types; virtual functions select behaviour at runtime.
- **Abstraction:** the GUI interacts with a question through common methods such as `prompt()`, `options()`, and `isCorrect()` rather than accessing its internal representation.

## 7. Templates and STL
- `ScoreKeeper<T>` is a class template for reusable scoring logic.
- `clampValue<T>` is a function template for keeping an index within valid bounds.
- `std::vector` stores the question bank and answer attempts.
- `std::unique_ptr` manages dynamically allocated question objects safely.
- `std::shuffle` randomizes the order of questions.
- `std::string` and `std::wstring` manage text.

## 8. Exception and file handling
- The app reads question entries from `questions.txt` using `std::ifstream`.
- If the file is absent or contains no valid questions, the app uses built-in sample questions.
- Invalid question records are skipped rather than terminating the whole application.
- At the end of a quiz, the app appends the timestamp, score, and percentage to `results.txt` using `std::ofstream`.
- If saving fails, the app displays a warning instead of crashing.

## 9. Algorithm
1. Start the application and load the question bank.
2. If loading fails, use the built-in question bank.
3. Shuffle the questions and display the first question.
4. Wait for the user to choose an option and select **Check Answer**.
5. Compare the selected option with the stored correct answer, update the score, and display feedback.
6. Move to the next question and repeat until all questions are answered.
7. Display the final score and a brief review of incorrect answers.
8. Append the result to `results.txt` and allow the user to restart or close the app.

## 10. Course outcome mapping

| CO | Evidence in the project |
|---|---|
| CO1 | Class-based design, encapsulated question data, quiz state, and scoring behaviour. |
| CO2 | `MultipleChoiceQuestion` and `TrueFalseQuestion` reuse the `Question` base class. |
| CO3 | Virtual functions and base-class references/pointers support different question types at runtime. |
| CO4 | Reading and writing files, validation of question records, exception handling, and fallback behaviour. |
| CO5 | `ScoreKeeper<T>`, `clampValue<T>`, and STL containers/algorithms. |

## 11. Test cases

| Test | Expected result |
|---|---|
| Select a correct answer | The app shows a correct message and increases the score. |
| Select a wrong answer | The app shows the correct option and an explanation. |
| Continue through all questions | A final score and incorrect-answer review appear. |
| Run without `questions.txt` | The app opens using built-in sample questions. |
| Complete a quiz with write access | A result line is appended to `results.txt`. |
| Complete a quiz without write access | The quiz completes and displays a save warning. |
| Restart the quiz | The score resets and the question order is shuffled again. |

## 12. Limitations and future scope
- The current version is a local, single-user quiz and stores results as plain text.
- Future versions could add categories, difficulty selection, a timer, user profiles, a database, and a cross-platform GUI framework.

## 13. Conclusion
OOP Quiz Studio is a practical C++ project that combines a simple GUI with object-oriented design, reusable inheritance, runtime polymorphism, file and exception handling, templates, and STL. The project demonstrates all five course outcomes in one small application.
