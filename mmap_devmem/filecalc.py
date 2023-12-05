def calculate_file_stats(file_path):
    try:
        with open(file_path, 'r') as file:
            num_characters = 0
            num_A = 0
            num_B = 0
            num_sp = 0
            for char in file.read():
                num_characters += 1

                if char == 'A':
                    num_A += 1
                elif char == 'B':
                    num_B += 1
                elif char.isspace():
                    num_sp += 1

        return num_characters, num_A, num_B, num_sp

    except FileNotFoundError:
        return None  # File not found
    except Exception as e:
        print(f"Error: {e}")
        return None

base_path = "/home/dante/work/hugetable_task/mmap_devmem/"
def file_calculation(file_path):
    print(f"Calculating file statistics for {base_path + file_path}...")
    file_stats = calculate_file_stats(file_path)
    if file_stats is None:
        print("File not found")
    else:
        print(f"Number of bytes: {file_stats[0]/(1024*1024)} MB")
        print(f"Number of A's: {file_stats[1]/(1024*1024)} MB")
        print(f"Number of B's: {file_stats[2]/(1024*1024)} MB")

file_calculation("output_0")
file_calculation("output_1")
file_calculation("output_2")
file_calculation("output_3")
file_calculation("output_4")
print(f"Calculating file statistics for hugepage...")
file_stats = calculate_file_stats("/mnt/hugetlbfs/hugepage")
if file_stats is None:
    print("File not found")
else:
    print(f"Number of bytes: {file_stats[0]/(1024*1024)} MB")
    print(f"Total size of A's: {file_stats[1]/(1024*1024)} MB")
    print(f"Total size of B's: {file_stats[2]/(1024*1024)} MB")
