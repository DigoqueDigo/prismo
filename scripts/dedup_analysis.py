import sys
from collections import Counter
from typing import List, Tuple
from prismo_entry import PrismoEntry
from dataclasses import dataclass
from tabulate import tabulate  # type: ignore

@dataclass
class BlockStatsEntry:
    repeats: int
    unique_blocks: int
    percentage_unique: float
    operations: int
    percentage_global: float

    def to_tuple(self) -> Tuple[int, int, float, int, float]:
        return (
            self.repeats,
            self.unique_blocks,
            self.percentage_unique,
            self.operations,
            self.percentage_global
        )

@dataclass
class WriteStatistics:
    entries: List[BlockStatsEntry]
    total_operations: int
    unique_blocks: int


def get_prismo_entries(log_filename: str) -> List[PrismoEntry]:
    with open(log_filename, 'r') as log_file:
        return [PrismoEntry(line) for line in log_file if line.strip()]


def compute_write_statistics(entries: List[PrismoEntry]) -> WriteStatistics:
    write_blocks: List[int] = [entry.block for entry in entries if entry.type == 1]
    repetitions_by_block: Counter[int] = Counter(write_blocks)
    summary: Counter[int] = Counter(repetitions_by_block.values())

    total_operations: int = len(entries)
    unique_blocks: int = len(repetitions_by_block)

    data: List[BlockStatsEntry] = []
    for repeats, count in summary.items():
        data.append(BlockStatsEntry(
            repeats=repeats - 1,
            unique_blocks=count,
            percentage_unique=round(count / unique_blocks * 100, 2) if unique_blocks else 0,
            operations=repeats * count,
            percentage_global=round(repeats * count / total_operations * 100, 2) if total_operations else 0
        ))

    data.sort(key=lambda x: x.repeats)

    return WriteStatistics(entries=data, total_operations=total_operations, unique_blocks=unique_blocks)


def print_statistics(log_file: str, stats: WriteStatistics):
    headers = [
        'Repeats',
        'Unique blocks',
        'Percentage unique',
        'Operations',
        'Percentage global'
    ]

    table_data: List[Tuple[int, int, float, int, float]] = [entry.to_tuple() for entry in stats.entries]
    avg_access = stats.total_operations / stats.unique_blocks if stats.unique_blocks else 0

    print(tabulate(table_data, headers=headers, tablefmt='rounded_outline'))

    print(f'\nSummary: {log_file}')
    print(f'  {"Total operations (log lines)":<30}: {stats.total_operations}')
    print(f'  {"Total writes":<30}: {sum(entry.operations for entry in stats.entries)}')
    print(f'  {"Unique blocks":<30}: {stats.unique_blocks}')
    print(f'  {"Average accesses per block":<30}: {avg_access:.3f}')


def dedup_analysis(log_filename: str) -> None:
    entries: List[PrismoEntry] = get_prismo_entries(log_filename)
    write_stats: WriteStatistics = compute_write_statistics(entries)
    print_statistics(log_filename, write_stats)


if __name__ == '__main__':
    for log_filename in sys.argv[1:]:
        dedup_analysis(log_filename)