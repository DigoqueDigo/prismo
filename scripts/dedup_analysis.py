import re
import sys
from collections import Counter
from tabulate import tabulate # type: ignore


def get_blocks_ids(log_filename: str) -> list[int]:
    block_ids: list[int] = []

    with open(log_filename, 'r') as log_file:
        for log_line in log_file:
            match = re.search(r"block=(\d+)", log_line)
            if match:
                block_id = int(match.group(1))
                block_ids.append(int(block_id))

    return block_ids


if __name__ == '__main__':
    

    log_files: list[str] = sys.argv[1:]

    for log_file in log_files:
        blocks_ids: list[int] = get_blocks_ids(log_file)
        repetitions_by_block: Counter[int] = Counter(blocks_ids)
        summary: Counter[int] = Counter(repetitions_by_block.values())

        total_operations: int = len(blocks_ids)
        unique_blocks: int = len(repetitions_by_block)

        data: list[tuple[int, int, float, int, float]] = []
        headers: list[str] = [
            "Repeats",
            "Unique blocks",
            "Percentage unique",
            "Operations",
            "Percentage global"
        ]

        for repeats, count in summary.items():
            data.append((
                repeats - 1,
                count,
                round(count / unique_blocks * 100, 2),
                repeats * count,
                round(repeats * count / total_operations * 100, 2),
            ))

        data_sorted = sorted(data, key=lambda x: x[0])
        print(tabulate(data_sorted, headers=headers, tablefmt='rounded_outline'))

        print("Summary:")
        print(f"  {'Total operations (log lines)':<30}: {total_operations}")
        print(f"  {'Unique blocks':<30}: {unique_blocks}")
        print(f"  {'Average accesses per block':<30}: {total_operations/unique_blocks:.3f}")
