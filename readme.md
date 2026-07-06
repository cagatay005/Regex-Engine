# C Regex Engine (Pike VM / State-Set Simulation)

A fully-featured, industrial-grade Regular Expression engine written entirely from scratch in C. 

Unlike standard recursive backtracking engines that suffer from exponential time complexity on complex patterns, this project implements a **State-Set Simulation (Pike VM)** architecture. This allows the engine to process strings with a strict $O(N)$ time complexity, making it completely immune to "Catastrophic Backtracking".

---

## Core Features

*   **Custom Compiler Architecture:** Features a hand-written Lexer (Tokenization) and Parser (AST/Expression resolution) that strictly respects operator precedence.
*   **NFA Graph Generation:** Converts regex tokens into a Non-deterministic Finite Automaton (NFA).
*   **Pike VM (State-Set Matching):** Traverses the NFA using parallel states, eliminating the need for backtracking.
*   **Capture Groups & Extraction:** Fully supports data extraction via capture groups `(...)` with industry-standard **Greedy Matching**.
*   **Custom CLI:** An interactive, Matrix-Green styled terminal interface with multilingual support.

## Supported Regex Syntax

The engine supports a wide range of standard regular expression operations:

*   **Quantifiers:** `*` (0 or more), `+` (1 or more), `?` (0 or 1), `{n,m}` (range repetition)
*   **Alternation & Grouping:** `|` (OR operator), `(...)` (Capture Groups)
*   **Character Classes:** `[abc]`, `[a-zA-Z0-9]`, `[^0-9]` (Negative classes)
*   **Anchors:** `^` (Start of string), `$` (End of string)
*   **Escape Sequences:** `\d` (digits), `\w` (word characters), `\s` (whitespace)
*   **Wildcard:** `.` (Any character except newline)
*   **Literal Escapes:** `\*`, `\+`, `\(`, etc.

## Installation & Compilation

This project is written in standard C99. You will need `gcc` and `make` (or `mingw32-make` on Windows) installed on your system.

**1. Clone the repository:**
```bash
git clone https://github.com/cagatay005/regex-engine.git
cd regex-engine
```

**2. Compile the project:**
_Using MinGW on Windows:_
```bash
mingw32-make
```
*(On Linux/Mac, simply run `make` depending on your Makefile setup).*

**3. Run the engine:**
```bash
./regex_engine
```

## Usage & CLI Commands

_Once the engine is running, you will be greeted by the interactive terminal. You can enter a regex pattern, followed by the text you want to test._

**Available CLI Commands:**

*   **code:** Displays the help menu and command list.
*   **clear:** Clears the terminal screen.
*   **en:** Switches the interface language to English.
*   **tr:** Switches the interface language to Turkish.
*   **q:** Quits the application gracefully.

---
---

# C Regex Motoru (Pike VM / State-Set Simülasyonu)

Sıfırdan, tamamen C dili ile yazılmış, endüstriyel standartlarda bir Düzenli İfade (Regex) motoru.

Karmaşık desenlerde üstel (exponential) zaman karmaşıklığı çöküşleri yaşayan standart özyinelemeli (recursive) motorların aksine, bu proje **State-Set Simülasyonu (Pike VM)** mimarisi kullanılarak geliştirilmiştir. Bu sayede motor, metinleri kusursuz bir $O(N)$ zaman karmaşıklığıyla işler ve "Felaketsel Geri İzleme" (Catastrophic Backtracking) sorununu tamamen ortadan kaldırır.

---

## Temel Özellikler

*   **Özel Derleyici Mimarisi:** İşlem önceliğine tam uyum sağlayan, el yapımı bir Lexer (Kelime İşlemci) ve Parser (Ayrıştırıcı) içerir.
*   **NFA Ağaç Üretimi:** Regex token'larını Belirsiz Sonlu Otomat (NFA) yapısına çevirir.
*   **Pike VM (Durum-Kümesi Eşleştirme):** Geri dönme (backtracking) ihtiyacını ortadan kaldırarak NFA üzerinde paralel klonlarla ilerler.
*   **Grup Yakalama ve Veri Ayıklama:** Parantez `(...)` yapılarıyla metin ayıklamayı ve endüstri standardı olan **Açgözlü Eşleşme (Greedy Matching)** mantığını tam olarak destekler.
*   **Gelişmiş CLI:** Çoklu dil desteğine sahip, Matrix Yeşili temalı interaktif bir terminal arayüzü sunar.

## Desteklenen Regex Sözdizimi (Syntax)

Motor, endüstri standartlarındaki temel regex operatörlerini destekler:

*   **Kuantörler (Miktar Belirticiler):** `*` (0 veya daha fazla), `+` (1 veya daha fazla), `?` (0 veya 1), `{n,m}` (belirli aralıkta tekrar)
*   **Alternatif ve Gruplama:** `|` (VEYA operatörü), `(...)` (Yakalama Grupları)
*   **Karakter Sınıfları:** `[abc]`, `[a-zA-Z0-9]`, `[^0-9]` (Negatif/Dışlayıcı sınıflar)
*   **Çapalar:** `^` (Metin başlangıcı), `$` (Metin bitişi)
*   **Kaçış Dizileri:** `\d` (rakamlar), `\w` (kelime karakterleri), `\s` (boşluklar)
*   **Joker Karakter:** `.` (Yeni satır hariç herhangi bir karakter)
*   **Özel Karakter Kaçışları:** `\*`, `\+`, `\(`, vb.

## Kurulum ve Derleme

Bu proje standart C99 ile yazılmıştır. Sisteminizde `gcc` ve `make` (Windows için `mingw32-make`) kurulu olması gerekmektedir.

**1. Projeyi klonlayın:**
```bash
git clone https://github.com/cagatay005/regex-engine.git
cd regex-engine
```

**2. Projeyi derleyin:**
_Windows üzerinde MinGW kullanarak:_
```bash
mingw32-make
```
*(Linux/Mac sistemlerinde Makefile ayarınıza göre doğrudan `make` komutunu kullanabilirsiniz).*

**3. Motoru çalıştırın:**
```bash
./regex_engine
```

## Kullanım ve CLI Komutları

Motor çalıştırıldığında sizi interaktif bir terminal karşılar. Önce bir regex deseni, ardından eşleşmesini istediğiniz metni girerek sistemi test edebilirsiniz.

**Kullanılabilir CLI Komutları:**

*   **code:** Yardım menüsünü ve komut listesini gösterir.
*   **clear:** Terminal ekranını temizler.
*   **en:** Arayüz dilini İngilizceye çevirir.
*   **tr:** Arayüz dilini Türkçeye çevirir.
*   **q:** Sistemden güvenli bir şekilde çıkış yapar.
