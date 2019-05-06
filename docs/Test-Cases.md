# Test Cases for winLAME

## Use Cases

### Case: Encode audio files to 

Steps:

Expected result:

### Case: Read CD tracks and encode them to audio files

Steps:

Expected result:

## Miscellaneous tests

### Encoding files

Case: Use filename with characters in a local codepage (like umlauts)
Expected result: Encoded files should contain correct filenames.

Case: Try to convert invalid or non-audio files
Expected result: An error message that the file is invalid or no input module
could be found should be shown.

Case:
Expected result:

Case:
Expected result:

### Encoding CDs

Case: Try to encode audio CD that is scratched
Epected result:

Case: Try to encode Data CD
Expected result: Error is reported in 

Case: Insert CD after "Encoding CD" is selected
Expected result: After some seconds, the tracks need to be detected and
listed.

Case: Try to request FreeDB or CoverArt data without internet connection
Expected result: After a while an error message should be reported.

Case: Use characters in a local codepage (like umlauts) in the CD or Track
title or other metadata
Expected result: Encoded files should contain correct metadata (check with
e.g. Mp3Tag).

### Classic Mode

Case: Try cancelling encoding from Encoding page
Expected result: First, a message asking if tasks should be stopped should
appear, and when answering with Yes, the Classic mode start page should
appear.

Case: Watch progress of encoding in classic mode in Task Bar icon
Expected result: The task bar should show a progress background while encoding
files.

Case: Switch to modern mode while encoding in Classic mode
Epected result: Encoding should continue, showing the task list.
