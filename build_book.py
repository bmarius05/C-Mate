import argparse
from pathlib import Path
import chess.pgn


def process_pgn_folder(
    input_dir="pgns",
    output_file="book.txt",
    min_moves=4,
    max_moves=20,
    append_existing=True,
):
    folder_path = Path(input_dir)

    if not folder_path.exists() or not folder_path.is_dir():
        print(f"[EROARE] Folderul '{input_dir}' nu exista sau nu este director!")
        return

    # Gasim toate fisierele cu extensia .pgn (indiferent de majuscule/minuscule)
    pgn_files = list(folder_path.glob("*.pgn")) + list(folder_path.glob("*.PGN"))
    if not pgn_files:
        print(
            f"[AVERTISMENT] Nu s-au gasit fisiere .pgn in folderul '{input_dir}'."
        )
        return

    unique_lines = set()
    out_path = Path(output_file)

    # Daca vrem sa pastram si ce era deja in book.txt, incarcam liniile existente
    if append_existing and out_path.exists():
        with open(out_path, "r", encoding="utf-8") as f:
            for line in f:
                cleaned = line.strip()
                if cleaned:
                    unique_lines.add(cleaned)
        print(
            f"Am incarcat {len(unique_lines)} linii existente din '{output_file}'."
        )

    print(
        f"Am gasit {len(pgn_files)} fisier(e) PGN. Incepe procesarea...\n"
        + "-" * 50
    )

    total_games_read = 0

    for pgn_file in pgn_files:
        print(f"-> Citim: {pgn_file.name} ...")
        games_in_file = 0

        # Incercam utf-8-sig (standard), cu fallback la latin-1 daca PGN-ul are caractere speciale vechi
        try:
            f = open(pgn_file, "r", encoding="utf-8-sig")
        except UnicodeDecodeError:
            f = open(pgn_file, "r", encoding="latin-1")

        with f:
            while True:
                try:
                    game = chess.pgn.read_game(f)
                except Exception as e:
                    print(
                        f"   [Eroare citire in {pgn_file.name} la jocul {games_in_file + 1}]: {e}"
                    )
                    break

                if game is None:
                    break

                total_games_read += 1
                games_in_file += 1

                board = game.board()
                moves = []

                for move in game.mainline_moves():
                    moves.append(board.uci(move))
                    board.push(move)

                # Daca partida are mai puține mutari decat minimul cerut, o sarim
                if len(moves) < min_moves:
                    continue

                # Taiem linia la numărul maxim de semi-mutări dorit (ex: primele 20 de semi-mutări = 10 mutări complete)
                if max_moves is not None:
                    moves = moves[:max_moves]

                if moves:
                    unique_lines.add(" ".join(moves))

        print(f"   Am procesat {games_in_file} partide din {pgn_file.name}.")

    # Scriem tot setul unic in book.txt
    with open(output_file, "w", encoding="utf-8") as out:
        for line in unique_lines:
            out.write(line + "\n")

    print("-" * 50)
    print("FINISH!")
    print(f"Total fisiere PGN scanate: {len(pgn_files)}")
    print(f"Total partide citite: {total_games_read}")
    print(
        f"Total linii unice finale salvate in '{output_file}': {len(unique_lines)}"
    )


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Converteste toate PGN-urile dintr-un folder in UCI pentru C-Mate."
    )
    parser.add_argument(
        "-i",
        "--input-dir",
        default="pgns",
        help="Folderul cu fisiere .pgn (implicit: pgns)",
    )
    parser.add_argument(
        "-o",
        "--output",
        default="book.txt",
        help="Fisierul unde se salveaza/adauga liniile (implicit: book.txt)",
    )
    parser.add_argument(
        "--min-moves",
        type=int,
        default=4,
        help="Numar minim de semi-mutari per linie (implicit: 4)",
    )
    parser.add_argument(
        "--max-moves",
        type=int,
        default=20,
        help="Numar maxim de semi-mutari per linie (implicit: 20)",
    )
    parser.add_argument(
        "--no-append",
        action="store_false",
        dest="append_existing",
        help="Suprascrie book.txt fara a pastra liniile vechi",
    )

    args = parser.parse_args()

    process_pgn_folder(
        input_dir=args.input_dir,
        output_file=args.output,
        min_moves=args.min_moves,
        max_moves=args.max_moves,
        append_existing=args.append_existing,
    )