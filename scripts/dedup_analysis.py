import sys
from dataclasses import dataclass
from tabulate import tabulate  # type: ignore
from collections import Counter, defaultdict
from prismo_entry import PrismoEntry, get_prismo_entries

@dataclass
class CompressionStatsEntry:
    reduction: int
    percentage: float

    def __str__(self) -> str:
        return f'{self.percentage:.2f}% → {self.reduction}'

@dataclass
class DedupStatsEntry:
    repeats: int
    unique_blocks: int
    percentage_unique: float
    operations: int
    percentage_global: float
    compression_entries: list[CompressionStatsEntry]

    def to_tuple(self) -> tuple[int, int, float, int, float, str]:
        return (
            self.repeats,
            self.unique_blocks,
            self.percentage_unique,
            self.operations,
            self.percentage_global,
            ' '.join(
                str(cpr_entry)
                for cpr_entry in sorted(
                    self.compression_entries,
                    key=lambda cpr_entry: cpr_entry.reduction
                )
                if cpr_entry.percentage > 0.0
            )
        )

@dataclass
class WriteStatistics:
    entries: list[DedupStatsEntry]
    total_operations: int
    unique_blocks: int


def compute_write_statistics(entries: list[PrismoEntry]) -> WriteStatistics:
    write_entries: list[PrismoEntry] = [e for e in entries if e.type == 1]

    entries_by_block: dict[int, list[PrismoEntry]] = defaultdict(list)
    for e in write_entries:
        entries_by_block[e.block].append(e)

    repetitions_by_block: dict[int, int] = {
        block: len(block_entries)
        for block, block_entries in entries_by_block.items()
    }

    total_operations: int = len(entries)
    unique_blocks: int = len(repetitions_by_block)

    blocks_by_repeats: dict[int, list[int]] = defaultdict(list)
    for block, reps in repetitions_by_block.items():
        blocks_by_repeats[reps].append(block)

    data: list[DedupStatsEntry] = []

    for repeats, blocks in blocks_by_repeats.items():

        selected_entries = [
            e
            for block in blocks
            for e in entries_by_block[block]
        ]

        compression_counter = Counter(e.cpr for e in selected_entries)
        total_selected = len(selected_entries)

        compression_entries = [
            CompressionStatsEntry(
                reduction=reduction,
                percentage=round(cnt / total_selected * 100, 2)
            )
            for reduction, cnt in compression_counter.items()
        ]

        block_count = len(blocks)

        data.append(DedupStatsEntry(
            repeats=repeats - 1,
            unique_blocks=block_count,
            percentage_unique=round(block_count / unique_blocks * 100, 2) if unique_blocks else 0,
            operations=repeats * block_count,
            percentage_global=round(repeats * block_count / total_operations * 100, 2) if total_operations else 0,
            compression_entries=compression_entries
        ))

    data.sort(key=lambda x: x.repeats)

    return WriteStatistics(
        entries=data,
        total_operations=total_operations,
        unique_blocks=unique_blocks
    )


def print_statistics(log_filename: str, stats: WriteStatistics):
    headers = [
        'Repeats',
        'Unique blocks',
        'Percentage unique',
        'Operations',
        'Percentage global',
        'Compression distribution',
    ]

    table_data: list[tuple[int, int, float, int, float, str]] = [entry.to_tuple() for entry in stats.entries]
    avg_access = stats.total_operations / stats.unique_blocks if stats.unique_blocks else 0

    print(tabulate(table_data, headers=headers, tablefmt='rounded_outline'))

    print(f'\nSummary: {log_filename}')
    print(f'  {'Total operations (log lines)':<30}: {stats.total_operations}')
    print(f'  {'Total writes':<30}: {sum(entry.operations for entry in stats.entries)}')
    print(f'  {'Unique blocks':<30}: {stats.unique_blocks}')
    print(f'  {'Average accesses per block':<30}: {avg_access:.3f}')


def dedup_analysis(log_filename: str) -> None:
    entries: list[PrismoEntry] = get_prismo_entries(log_filename)
    write_stats: WriteStatistics = compute_write_statistics(entries)
    print_statistics(log_filename, write_stats)


if __name__ == '__main__':
    for log_filename in sys.argv[1:]:
        dedup_analysis(log_filename)