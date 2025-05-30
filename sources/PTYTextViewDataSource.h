// DataSource for PTYTextView.
#import "iTermColorMap.h"
#import "iTermCursor.h"
#import "iTermFindDriver.h"
#import "iTermLogicalMovementHelper.h"
#import "iTermTextDataSource.h"
#import "ScreenChar.h"
#import "LineBuffer.h"
#import "VT100Grid.h"
#import "VT100Terminal.h"

@class Interval;
@class iTermColorMap;
@class iTermExternalAttributeIndex;
@protocol iTermFoldMarkReading;
@class iTermKittyImageDraw;
@protocol iTermMark;
@class iTermOffscreenCommandLine;
@protocol iTermPathMarkReading;
@class iTermRange;
@class iTermRangeArray;
@class iTermSavedIntervalTreeObject;
@class iTermSearchEngine;
@class iTermTerminalButtonPlace;
@protocol IntervalTreeImmutableObject;
@class PTYAnnotation;
@protocol PTYAnnotationReading;
@class PTYNoteViewController;
@class PTYSession;
@class PTYTask;
@protocol Porthole;
@class SCPPath;
@class VT100Grid;
@protocol VT100RemoteHostReading;
@protocol VT100ScreenMarkReading;
@class VT100Terminal;

@protocol PTYTextViewSynchronousUpdateStateReading<NSObject>
@property (nonatomic, strong, readonly) id<VT100GridReading> grid;
@property (nonatomic, readonly) BOOL cursorVisible;
@property (nonatomic, strong, readonly) id<iTermColorMapReading> colorMap;
@end

@interface PTYTextViewSynchronousUpdateState : NSObject<PTYTextViewSynchronousUpdateStateReading, NSCopying>
@property (nonatomic, strong) VT100Grid *grid;
@property (nonatomic) BOOL cursorVisible;
@property (nonatomic, strong) iTermColorMap *colorMap;
@end

@protocol PTYTextViewDataSource <iTermLogicalMovementHelperDelegate, iTermTextDataSource>

- (BOOL)terminalReverseVideo;
- (MouseMode)terminalMouseMode;
- (VT100Output *)terminalOutput;
- (BOOL)terminalAlternateScrollMode;
- (BOOL)terminalSoftAlternateScreenMode;
- (BOOL)terminalAutorepeatMode;
- (int)height;

// Cursor position is 1-based (the top left is at 1,1).
- (int)cursorX;
- (int)cursorY;

// Provide a buffer as large as sizeof(screen_char_t*) * ([SCREEN width] + 1)
- (const screen_char_t *)getLineAtIndex:(int)theIndex withBuffer:(screen_char_t*)buffer;
- (NSArray<ScreenCharArray *> *)linesInRange:(NSRange)range;
- (int)numberOfScrollbackLines;
- (int)scrollbackOverflow;
- (long long)absoluteLineNumberOfCursor;

// Return a human-readable dump of the screen contents.
- (NSString*)debugString;
- (BOOL)isAllDirty;
- (void)setRangeOfCharsAnimated:(NSRange)range onLine:(int)line;
- (NSIndexSet *)animatedLines;
- (void)resetAnimatedLines;

// Check if any the character at x,y has been marked dirty.
- (BOOL)isDirtyAtX:(int)x Y:(int)y;
- (NSIndexSet *)dirtyIndexesOnLine:(int)line;

// Save the current state to a new frame in the dvr.
- (void)saveToDvr:(NSIndexSet *)cleanLines;

// If this returns true then the textview will broadcast iTermTabContentsChanged
// when a dirty char is found.
- (BOOL)shouldSendContentsChangedNotification;

// Smallest range that contains all dirty chars for a line at a screen location.
// NOTE: y is a grid index and cannot refer to scrollback history.
- (VT100GridRange)dirtyRangeForLine:(int)y;

// Returns the last modified date for a given line.
- (NSDate *)timestampForLine:(int)y;

- (void)addNote:(PTYAnnotation *)note inRange:(VT100GridCoordRange)range focus:(BOOL)focus visible:(BOOL)visible;

// Returns all notes in a range of cells.
- (NSArray<id<PTYAnnotationReading>> *)annotationsInRange:(VT100GridCoordRange)range;

- (VT100GridCoordRange)coordRangeOfAnnotation:(id<IntervalTreeImmutableObject>)note;
- (NSArray *)charactersWithNotesOnLine:(int)line;
- (id<VT100ScreenMarkReading>)screenMarkOnLine:(int)line;
- (id<iTermMark>)drawableMarkOnLine:(int)line;
- (void)removeNamedMark:(id<VT100ScreenMarkReading>)mark;
- (id<VT100ScreenMarkReading>)commandMarkAt:(VT100GridCoord)coord
                            mustHaveCommand:(BOOL)mustHaveCommand
                                      range:(out VT100GridWindowedRange *)range;
- (id<iTermPathMarkReading>)pathMarkAt:(VT100GridCoord)coord;
- (id<VT100ScreenMarkReading>)promptMarkAfterPromptMark:(id<VT100ScreenMarkReading>)predecessor;
- (id<VT100ScreenMarkReading>)promptMarkBeforePromptMark:(id<VT100ScreenMarkReading>)predecessor;

- (VT100GridAbsCoordRange)absCoordRangeForInterval:(Interval *)interval;
- (VT100GridCoordRange)coordRangeForInterval:(Interval *)interval;
- (Interval *)intervalForGridAbsCoordRange:(VT100GridAbsCoordRange)absCoordRange;

- (NSString *)workingDirectoryOnLine:(int)line;

- (SCPPath *)scpPathForFile:(NSString *)filename onLine:(int)line;
- (id<VT100RemoteHostReading>)remoteHostOnLine:(int)line;
- (VT100GridCoordRange)textViewRangeOfOutputForCommandMark:(id<VT100ScreenMarkReading>)mark;

// Indicates if we're in alternate screen mode.
- (BOOL)showingAlternateScreen;

- (void)clearBuffer;

// When the cursor is about to be hidden, a copy of the grid is saved. This
// method is used to temporarily swap in the saved grid if one is available. It
// calls `block` with a nonnil state if the saved grid was swapped in.
- (void)performBlockWithSavedGrid:(void (^)(id<PTYTextViewSynchronousUpdateStateReading> state))block;

- (NSString *)compactLineDumpWithContinuationMarks;
- (NSOrderedSet<NSString *> *)sgrCodesForChar:(screen_char_t)c
                           externalAttributes:(iTermExternalAttribute *)ea;

- (void)setColor:(NSColor *)color forKey:(int)key;
- (id<iTermColorMapReading>)colorMap;
- (void)removeAnnotation:(id<PTYAnnotationReading>)annotation;
- (void)setStringValueOfAnnotation:(id<PTYAnnotationReading>)annotation to:(NSString *)stringValue;

- (void)resetDirty;

- (id<iTermTextDataSource>)snapshotDataSource;
- (id<iTermTextDataSource>)snapshotWithPrimaryGrid;

- (void)replaceRange:(VT100GridAbsCoordRange)range
        withPorthole:(id<Porthole>)porthole
            ofHeight:(int)numLines;
- (void)replaceRange:(VT100GridAbsCoordRange)range
            withLine:(ScreenCharArray *)line
        promptLength:(NSInteger)promptLength;
- (void)replaceRange:(VT100GridAbsCoordRange)range
            withLines:(NSArray<ScreenCharArray *> *)lines
        promptLength:(NSInteger)promptLength
          blockMarks:(NSDictionary<NSString *, iTermRange *> *)blockMarks;
- (NSDictionary<NSString *, iTermRange *> *)blockMarkDictionaryOnLine:(long long)y;
- (BOOL)removeFoldsInRange:(NSRange)absRange;
- (NSIndexSet *)foldsInRange:(VT100GridRange)range;
- (NSArray<id<iTermFoldMarkReading>> *)foldMarksInRange:(VT100GridRange)range;

// includeSucessorDivider is the blank line before the next line-style mark
- (VT100GridAbsCoordRange)rangeOfCommandAndOutputForMark:(id<VT100ScreenMarkReading>)mark
                                  includeSucessorDivider:(BOOL)includeSucessorDivider;

- (void)replaceMark:(id<iTermMark>)mark
          withLines:(NSArray<ScreenCharArray *> *)lines
          savedITOs:(NSArray<iTermSavedIntervalTreeObject *> *)savedITOs;
- (void)changeHeightOfMark:(id<iTermMark>)mark to:(int)newHeight;
- (id<VT100ScreenMarkReading>)commandMarkAtOrBeforeLine:(int)line;

- (VT100GridCoordRange)coordRangeOfPorthole:(id<Porthole>)porthole;
- (iTermOffscreenCommandLine *)offscreenCommandLineBefore:(int)line;
- (NSInteger)numberOfCellsUsedInRange:(VT100GridRange)range;
- (NSArray<iTermTerminalButtonPlace *> *)buttonsInRange:(VT100GridRange)range;
- (VT100GridCoordRange)rangeOfBlockWithID:(NSString *)blockID;
- (NSArray<iTermKittyImageDraw *> *)kittyImageDraws;
- (void)addSavedIntervalTreeObjects:(NSArray<iTermSavedIntervalTreeObject *> *)savedITOs
                           baseLine:(long long)baseLine;
- (iTermSearchEngine *)searchEngine;
- (iTermBidiDisplayInfo *)bidiInfoForLine:(int)line;

@end
