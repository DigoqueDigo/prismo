import re
from datetime import datetime
from dataclasses import dataclass
from typing import ClassVar, List

@dataclass
class PrismoEntry:
    timestamp: datetime
    module: str
    level: str

    type: int
    block: int
    cpr: int
    sts: int
    ets: int
    pid: int
    tid: int
    req: int
    proc: int
    offset: int
    ret: int
    errno: int

    _FIELDS: ClassVar[List[str]] = [
        'type', 'block', 'cpr', 'sts', 'ets',
        'pid', 'tid', 'req', 'proc', 'offset', 'ret', 'errno'
    ]

    def __init__(self, line: str) -> None:
        header_match = re.match(r'\[(.*?)\] \[(.*?)\] \[(.*?)\] \[(.*)\]', line)
        if not header_match:
            raise ValueError(f'Invalid log line format: {line}')

        timestamp_str, module, level, fields_str = header_match.groups()
        self.timestamp = datetime.strptime(timestamp_str, '%Y-%m-%d %H:%M:%S.%f')
        self.module = module
        self.level = level

        fields: dict[str, int] = {}
        for part in fields_str.split():
            key, value = part.split('=')
            fields[key] = int(value)

        for field in self._FIELDS:
            if field not in fields:
                raise ValueError(f'Missing field "{field}" in log line: {line}')
            setattr(self, field, fields[field])


def get_prismo_entries(log_filename: str) -> List[PrismoEntry]:
    with open(log_filename, 'r') as log_file:
        return [PrismoEntry(line) for line in log_file if line.strip()]
