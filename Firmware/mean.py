def calculate_overall_mean(file_path):
    total = 0
    count = 0

    try:
        with open(file_path, 'r') as file:
            for line in file:
                try:
                    # Convert each line to a float and accumulate the total
                    total += float(line.strip())
                    count += 1
                except ValueError:
                    print(f"Warning: Skipping invalid value in line: '{line.strip()}'")

        if count > 0:
            overall_mean = total / count
            print(f"Overall Mean: {overall_mean:.2f}")
        else:
            print("No valid numbers found in the file.")
            
    except FileNotFoundError:
        print(f"Error: File '{file_path}' not found.")

# Replace 'input.txt' with the path to your file
file_path = 'log.csv'
calculate_overall_mean(file_path)
