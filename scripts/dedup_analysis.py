import argparse
import numpy as np
import matplotlib.pyplot as plt
from tabulate import tabulate
from dataclasses import dataclass
from collections import Counter, defaultdict
from prismo_entry import PrismoEntry, get_prismo_entries

@dataclass
class DedupAnalysisArgs:
    input: str

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
class Statistics:
    entries: list[DedupStatsEntry]
    total_operations: int
    unique_blocks: int


def get_entries_by_block(
    entries: list[PrismoEntry]
) -> dict[int, list[PrismoEntry]]:
    entries_by_block: dict[int, list[PrismoEntry]] = defaultdict(list)
    for e in entries:
        entries_by_block[e.block].append(e)
    return entries_by_block


def get_repetitions_by_block(
    entries_by_block: dict[int, list[PrismoEntry]]
) -> dict[int, int]:
    return {
        block: len(block_entries)
        for block, block_entries in entries_by_block.items()
    }


def calculate_write_statistics(
    entries: list[PrismoEntry],
    entries_by_block: dict[int, list[PrismoEntry]],
    repetitions_by_block: dict[int, int]
) -> Statistics:
    data: list[DedupStatsEntry] = []
    total_operations: int = len(entries)
    unique_blocks: int = len(repetitions_by_block)

    blocks_by_repeats: dict[int, list[int]] = defaultdict(list)
    for block, reps in repetitions_by_block.items():
        blocks_by_repeats[reps].append(block)

    for repeats, blocks in blocks_by_repeats.items():
        selected_entries = [
            e
            for block in blocks
            for e in entries_by_block[block]
        ]

        total_selected = len(selected_entries)
        compression_counter = Counter(e.cpr for e in selected_entries)

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

    return Statistics(
        entries=data,
        total_operations=total_operations,
        unique_blocks=unique_blocks
    )


def show_statistics_table(
    args: DedupAnalysisArgs,
    stats: Statistics
) -> None:
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

    print(f'\nSummary: {args.input}')
    print(f'  {'Total operations (log lines)':<30}: {stats.total_operations}')
    print(f'  {'Total writes':<30}: {sum(entry.operations for entry in stats.entries)}')
    print(f'  {'Unique blocks':<30}: {stats.unique_blocks}')
    print(f'  {'Average accesses per block':<30}: {avg_access:.3f}')


def plot_operations_vs_repeats(
    stats: Statistics
) -> None:
    repeats = [entry.repeats for entry in stats.entries]
    operations = [entry.operations for entry in stats.entries]
    unique_blocks = [entry.unique_blocks for entry in stats.entries]

    x = np.arange(len(repeats))

    fig, ax1 = plt.subplots(figsize=(10,6)) # type: ignore

    ax1.bar(x, operations, color='skyblue', label='Operations') # type: ignore
    ax1.set_xlabel('Number of Repeats') # type: ignore
    ax1.set_ylabel('Operations', color='black') # type: ignore
    ax1.tick_params(axis='y', labelcolor='black') # type: ignore
    ax1.set_xticks(x) # type: ignore
    ax1.set_xticklabels(repeats) # type: ignore
    ax1.grid(axis='y', linestyle='--', alpha=0.7) # type: ignore

    ax2 = ax1.twinx()
    ax2.plot(x, unique_blocks, color='brown', marker='o', label='Unique Blocks') # type: ignore
    ax2.set_ylabel('Unique Blocks', color='black') # type: ignore
    ax2.tick_params(axis='y', labelcolor='black') # type: ignore

    fig.tight_layout()
    fig.legend(loc='upper right', bbox_to_anchor=(1,1), bbox_transform=ax1.transAxes) # type: ignore

    plt.title('Operations and Unique Blocks vs Repeats') # type: ignore
    plt.savefig('png/operations_vs_repeats', dpi=300, bbox_inches='tight') # type: ignore
    plt.close()


def dedup_analysis(
    args: DedupAnalysisArgs
) -> None:
    write_entries: list[PrismoEntry] = [e for e in get_prismo_entries(args.input) if e.type == 1]
    entries_by_block: dict[int, list[PrismoEntry]] = get_entries_by_block(write_entries)
    repetition_by_block: dict[int, int] = get_repetitions_by_block(entries_by_block)

    write_stats: Statistics = calculate_write_statistics(
        write_entries,
        entries_by_block,
        repetition_by_block
    )

    plot_operations_vs_repeats(write_stats)
    show_statistics_table(args, write_stats)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        prog='dedup_analysis',
        description='Deduplication and compression distribution analysis',
    )

    parser.add_argument(
        '-i',
        '--input',
        type=str,
        required=True,
        help='prismo log file path'
    )

    args = parser.parse_args()
    args = DedupAnalysisArgs(input=args.input)

    dedup_analysis(args)