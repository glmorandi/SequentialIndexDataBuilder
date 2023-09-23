
# SequentialIndexDataBuilder

Creation of a sequential-indexed organization data file, for which 4 indexes will be built: two file indexes and two memory indexes.

## Installation

1. **Clone the Repository**

   Open your terminal/command prompt and run the following command:

   ```bash
   git clone https://github.com/glmorandi/SequentialIndexDataBuilder.git
   cd SequentialIndexDataBuilder
   ```

2. **Create a Virtual Environment (Optional)**

   Set up a virtual environment to isolate project dependencies:

   ```bash
   python -m venv venv
   source venv/bin/activate  # On Windows, use `venv\Scripts\activate`
   ```

3. **Install Dependencies**

   Install the project's Python dependencies using pip:

   ```bash
   pip install pandas
   ```

	Install GCC (on Debian/Ubuntu)
	```bash
	sudo apt update
	sudo apt install build-essential
	```
4. **Get the dataset**
Download the dataeset from Kaggle and place it on the same directory as ``main.py`` and ``main.c``:
[https://www.kaggle.com/datasets/gauthamp10/google-playstore-apps](https://www.kaggle.com/datasets/gauthamp10/google-playstore-apps)

6. **Run python script to clean the dataset**
   ```bash
   python main.py
   ```
7. **Compile the program**
	```bash
	gcc main.c -o main
	```
8. **Run the program**
	```bash
	./main
	```
