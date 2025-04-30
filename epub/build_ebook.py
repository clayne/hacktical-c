# pylint: disable=missing-module-docstring
# pylint: disable=missing-function-docstring
# pylint: disable=missing-class-docstring
# pylint: disable=line-too-long
# pylint: disable=consider-using-f-string

import argparse
import dataclasses
import json
import logging
import os
import pathlib
import re
import subprocess
import sys

@dataclasses.dataclass
class BuildConfig:
    """
    the collection of cli options used to configure this build
    """
    output_dir: pathlib.Path
    output_int_dir: pathlib.Path
    output_epub_path: pathlib.Path
    epub_metadata_path: pathlib.Path
    css_path: pathlib.Path
    chapters_data_json: pathlib.Path
    chapters_root: pathlib.Path

@dataclasses.dataclass
class ChapterData:
    """
    the files which are used to populate each section of the book
    """
    chapter_root: pathlib.Path
    code_supplements: list[pathlib.Path]

PANDOC_EXE_NAME = "pandoc"

def main() -> None:
    """
    build the ebook
    """
    logging.basicConfig(
        encoding='utf-8',
        level=logging.INFO,
        format='[%(asctime)s] [%(levelname)s] %(message)s',
        datefmt='%I:%M:%S')

    verify_pandoc_installed()

    parser = argparse.ArgumentParser(
        prog='build_ebook',
        description='script to collate and format markdown book chapters into an EPUB ebook')
    parser.add_argument(
        '--chapters-root',
        default=None,
        help='The directory where chapter data is stored. If none provided <repo>/ is used')
    parser.add_argument(
        '-o', '--output-epub-name',
        default='HackticalC.epub',
        help='The name of the generated EPUB book file')
    parser.add_argument(
        '--output-directory',
        default=None,
        help='The directory where generated output is placed. If none provided <repo>/epub/output used')
    parser.add_argument(
        '-v', '--verbose',
        default=False,
        action='store_true',
        help='whether to log output verbosely or not')

    # setup the log output
    parsed_args = parser.parse_args()
    if parsed_args.verbose:
        logging.getLogger().setLevel(logging.DEBUG)

    # parse our build configuration
    config = parse_config(parsed_args)
    logging.debug("output directory: %s", config.output_dir)
    logging.debug("intermediate output directory: %s", config.output_int_dir)
    logging.debug("output epub path: %s", config.output_epub_path)
    logging.debug("epub metadata path: %s", config.epub_metadata_path)
    logging.debug("book css path: %s", config.css_path)
    logging.debug("chapters data json path: %s", config.chapters_data_json)
    logging.debug("chapters root: %s", config.chapters_root)

    # ensure our output directories exist
    os.makedirs(config.output_dir, exist_ok=True)
    logging.debug("setup %s", config.output_dir)
    os.makedirs(config.output_int_dir, exist_ok=True)
    logging.debug("setup %s", config.output_int_dir)

    # read the book data from our JSON
    with open(config.chapters_data_json, encoding="utf8") as f:
        chapter_data = parse_chapter_data_from_json_blob(f.read())

    # expand the paths to absolute paths for easier debugging
    chapter_data = [ expand_chapter_data_paths(config.chapters_root, ch) for ch in chapter_data ]
    logging.debug("chapter data: %s", chapter_data)

    # take the original source chapters and slightly reformat them for nicer ebook presentation
    formatted_chapter_file_paths: list[pathlib.Path] = []
    for (i, ch) in enumerate(chapter_data):
        (chapter_main, chapter_supplement) = reformat_chapter_data(config.output_int_dir, i+1, ch)
        formatted_chapter_file_paths.append(chapter_main)
        if chapter_supplement is not None:
            formatted_chapter_file_paths.append(chapter_supplement)
    logging.debug("formatted chapters files: %s", formatted_chapter_file_paths)


    # throw the reformatted source .md files through pandoc to generate the ebook
    try:
        pandoc_args = [
            PANDOC_EXE_NAME,
            "-o", str(config.output_epub_path),
            "--css", str(config.css_path),
            "--toc", # auto-generate ToC
            "--metadata-file", str(config.epub_metadata_path),
            ] + [str(p) for p in formatted_chapter_file_paths]
        logging.debug("generating %s: cmd=%s", config.output_epub_path, pandoc_args)

        subprocess.run(pandoc_args, check=True)
        logging.info("generated %s", config.output_epub_path)
    except subprocess.CalledProcessError as ex:
        logging.error("Pandoc failed! retcode=%d", ex.returncode)
        logging.error("    cmd: %s", ex.cmd)
        logging.error("    args: %s", ex.args)
        logging.error("    stdout: %s", ex.stdout)
        logging.error("    stderr: %s", ex.stderr)

def parse_config(args: argparse.Namespace) -> BuildConfig:
    """
    read the cli args and default build settings into a build config
    """
    if args.output_directory is not None:
        output_dir = pathlib.Path(args.output_directory)
    else:
        output_dir = pathlib.Path(os.path.abspath(__file__)).parent / "output"

    if args.chapters_root is not None:
        chapters_root = pathlib.Path(args.chapters_root).resolve()
    else:
        chapters_root = pathlib.Path(os.path.abspath(__file__)).parent.parent

    # normalize our paths to nicer, "full" paths without relative indirection for ease of debugging
    script_dir = pathlib.Path(os.path.abspath(__file__)).parent.resolve()
    output_dir = output_dir.resolve()
    output_int_dir = output_dir / "int"
    output_epub_path = output_dir / args.output_epub_name
    epub_metadata_path = script_dir / "metadata.yml"
    book_css_path = script_dir / "styles.css"
    chapters_data_json_path = script_dir / "book_data.json"
    chapters_root = chapters_root.resolve()

    return BuildConfig(
        output_dir=output_dir,
        output_int_dir=output_int_dir,
        output_epub_path=output_epub_path,
        epub_metadata_path=epub_metadata_path,
        css_path=book_css_path,
        chapters_data_json=chapters_data_json_path,
        chapters_root=chapters_root)

def parse_chapter_data_from_json_blob(json_file_data: str) -> list[ChapterData]:
    """
    read the book data json blob to get all of the info we need to read the source chapter files
    """
    chapters_data = json.loads(json_file_data)

    if not isinstance(chapters_data, list):
        raise RuntimeError("Chapter data JSON malformed: expected JSON list")

    parsed_chapters_data: list[ChapterData] = []
    for chapter_data in chapters_data:
        if not isinstance(chapter_data, dict):
            raise RuntimeError("Chapter data JSON malformed: each entry in list must be chapter data object")

        if "chapter_root" not in chapter_data:
            raise RuntimeError("Chapter data JSON malformed: missing required 'chapter_root' in chapter")

        chapter_root = chapter_data["chapter_root"]
        if not isinstance(chapter_root, str):
            raise RuntimeError(f"Chapter data JSON malformed: 'chapter_root' is {type(chapter_root)}; expected str")

        code_supplements = chapter_data.get("code_supplements", [])
        for (i, supp)in enumerate(code_supplements):
            if not isinstance(supp, str):
                raise RuntimeError(f"Chapter data JSON malformed: code_supplements[{i}] was {type(supp)}; expected str")

        parsed_data = ChapterData(
            chapter_root=pathlib.Path(chapter_root),
            code_supplements=[ pathlib.Path(supp) for supp in code_supplements ])

        logging.debug("parsed chapter data #%d: %s", len(parsed_chapters_data), parsed_data)
        parsed_chapters_data.append(parsed_data)
    return parsed_chapters_data

def expand_chapter_data_paths(chapters_root: pathlib.Path, data: ChapterData) -> ChapterData:
    """
    build the actual paths to each of the chapter files from the chapter root
    """
    new_chapter_root = chapters_root / data.chapter_root
    return ChapterData(
        chapter_root=new_chapter_root,
        code_supplements=[ new_chapter_root/supp for supp in data.code_supplements ])

CHAPTER_TITLE_LINE_RE = re.compile(r"^[#]+\s+(\S+.*)")
def reformat_chapter_data(
    output_int_path: pathlib.Path,
    chapter_number: int,
    chapter_data: ChapterData
    ) -> tuple[pathlib.Path, pathlib.Path|None]:
    """
    reformat the chapter data for nicer presentation in the ebook
    - make the each chapter heading an H1 tag
    - collate the supplemental code files into their own "appendix" chapter
    """

    logging.debug("formatting chapter %d: %s", chapter_number, str(chapter_data.chapter_root))

    main_chapter_file = chapter_data.chapter_root / "README.md"
    with open(main_chapter_file, encoding="utf8") as f:
        reformatted_chapter_content_lines = [ line.rstrip() for line in f.readlines() ]
        if len(reformatted_chapter_content_lines) < 1:
            raise RuntimeError(f"missing title line in {main_chapter_file}")
        chapter_title_line_match = CHAPTER_TITLE_LINE_RE.match(reformatted_chapter_content_lines[0])
        if chapter_title_line_match is None:
            raise RuntimeError(f"first line isn't title line in {main_chapter_file}: {reformatted_chapter_content_lines[0]}")

        section_name: str = chapter_title_line_match.group(1)
        new_chapter_title_line = "# Ch %d. %s" % (chapter_number, section_name)

        reformatted_chapter_content_lines[0] = new_chapter_title_line

    section_name_file_id = section_name.replace(" ", "_")
    chapter_file_prefix = f"CHAPTER_{chapter_number:02d}_{section_name_file_id}"
    reformatted_chapter_file_path = output_int_path / f"{chapter_file_prefix}_MAIN.md"

    write_file_lines(reformatted_chapter_file_path, reformatted_chapter_content_lines)
    logging.debug("generated %s", reformatted_chapter_file_path)

    if len(chapter_data.code_supplements) > 0:
        supplement_chapter_title = f"# Supplements: {section_name}"
        new_chapter_supplement_path = output_int_path / f"{chapter_file_prefix}_SUPP.md"

        supplement_chapter_lines = []

        supplement_chapter_lines.append(supplement_chapter_title)
        for supp in chapter_data.code_supplements:
            supplement_chapter_lines.append("")

            supplement_chapter_lines.append(f"## {supp.name}") # add the supp sub-section title line
            supplement_chapter_lines.append("")

            # add a codeblock for each of the injected code supplement files
            supplement_chapter_lines.append("```C")
            with open(supp, encoding="utf8") as f:
                supplement_chapter_lines += ( line.rstrip() for line in f.readlines() )
            supplement_chapter_lines.append("```")

        write_file_lines(new_chapter_supplement_path, supplement_chapter_lines)
        logging.debug("generated %s", new_chapter_supplement_path)

    else:
        new_chapter_supplement_path = None

    return (reformatted_chapter_file_path, new_chapter_supplement_path)

def write_file_lines(filepath: pathlib.Path, lines: list[str], line_separator='\n', encoding="utf8"):
    """
    a quick helper to add normalized line endings and write out a collection of lines to a file
    """
    with open(filepath, "w", encoding=encoding) as f:
        f.writelines(line + line_separator for line in lines)

def verify_pandoc_installed() -> None:
    """
    ensure pandoc is available and working
    """
    pandoc_args = [ PANDOC_EXE_NAME, "--version" ]
    try:
        subprocess.run(pandoc_args, check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        logging.debug("verified pandoc installation")
    except subprocess.CalledProcessError as ex:
        logging.error("Pandoc installation issue! retcode=%d", ex.returncode)
        logging.error("    cmd: %s", ex.cmd)
        logging.error("    args: %s", ex.args)
        logging.error("    stdout: %s", ex.stdout)
        logging.error("    stderr: %s", ex.stderr)
        sys.exit()
    # pylint: disable=broad-exception-caught
    except Exception as ex:
        logging.error("Pandoc installation issue! %r", ex)
        logging.error("    tested with %s", pandoc_args)
        sys.exit()

if __name__ == "__main__":
    main()
