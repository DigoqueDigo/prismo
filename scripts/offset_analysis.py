import argparse
import seaborn as sns
import matplotlib.pyplot as plt
from dataclasses import dataclass
from prismo_entry import PrismoEntry, get_prismo_entries

@dataclass
class OffsetAnalysisArgs:
    input: str
    output: str
    bins: int


def plot_offset_distribuition(entries: list[PrismoEntry], args: OffsetAnalysisArgs) -> None:
    sns.histplot(
        [e.offset for e in entries],
        bins=args.bins,
        stat="density",
        kde=True,
        kde_kws={"bw_adjust": 1.5}
    )

    plt.xlabel("Offset") # type: ignore
    plt.ylabel("Density") # type: ignore
    plt.title("Offsets Distribution") # type: ignore

    plt.grid(alpha=0.3) # type: ignore
    plt.tight_layout()

    plt.savefig(args.output, dpi=300, bbox_inches='tight') # type: ignore
    plt.close()


def offset_analysis(args: OffsetAnalysisArgs) -> None:
    entries: list[PrismoEntry] = get_prismo_entries(args.input)
    plot_offset_distribuition(entries, args)


if __name__ == '__main__':

    parser = argparse.ArgumentParser(
        prog='offset_analysis',
        description='Offset distribution analysis',
    )

    parser.add_argument(
        '-i',
        '--input',
        type=str,
        required=True,
        help='prismo log file path'
    )

    parser.add_argument(
        '-o',
        '--output',
        type=str,
        required=True,
        help='png output file path'
    )

    parser.add_argument(
        '-b',
        '--bins',
        type=int,
        default=10,
        required=False,
        help='number of bins'
    )

    args = parser.parse_args()
    args = OffsetAnalysisArgs(
        input=args.input,
        output=args.output,
        bins=args.bins
    )

    offset_analysis(args)
