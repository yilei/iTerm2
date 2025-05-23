//
//  TmuxHistoryParser.m
//  iTerm
//
//  Created by George Nachman on 11/29/11.
//

#import "TmuxHistoryParser.h"

#import "iTermMalloc.h"
#import "ScreenChar.h"
#import "VT100Terminal.h"

@implementation TmuxHistoryParser

+ (TmuxHistoryParser *)sharedInstance
{
    static TmuxHistoryParser *instance;
    if (!instance) {
        instance = [[TmuxHistoryParser alloc] init];
    }
    return instance;
}

// Returns nil on error
// TODO: Test with italics
// TODO: Add external attributes here when tmux gains OSC 8 or colored underline support.
// For underline color, see https://github.com/tmux/tmux/issues/2928
- (NSData *)dataForHistoryLine:(NSString *)hist
                  withTerminal:(VT100Terminal *)terminal
        ambiguousIsDoubleWidth:(BOOL)ambiguousIsDoubleWidth
                unicodeVersion:(NSInteger)unicodeVersion
               alternateScreen:(BOOL)alternateScreen
                      rtlFound:(BOOL *)rtlFoundPtr {
    if (rtlFoundPtr) {
        *rtlFoundPtr = NO;
    }
    screen_char_t *screenChars;
    NSMutableData *result = [NSMutableData data];
    NSData *histData = [hist dataUsingEncoding:NSUTF8StringEncoding];
    [terminal.parser putStreamData:histData.bytes
                            length:histData.length];

    CVector vector;
    CVectorCreate(&vector, 100);
    [terminal.parser addParsedTokensToVector:&vector];
    int n = CVectorCount(&vector);
    for (int i = 0; i < n; i++) {
        VT100Token *token = CVectorGetObject(&vector, i);
        [terminal executeToken:token];
        NSString *string = token.isStringType ? token.string : nil;
        if (!string && (token->type == VT100_ASCIISTRING ||
                        token->type == VT100_MIXED_ASCII_CR_LF ||
                        token->type == VT100_GANG)) {
            string = [token stringForAsciiData];
        }

        if (string) {
            // Allocate double space in case they're all double-width characters.
            screenChars = iTermMalloc(sizeof(screen_char_t) * 2 * string.length);
            int len = 0;
            BOOL rtlFound = NO;
            StringToScreenChars(string,
                                screenChars,
                                [terminal foregroundColorCode],
                                [terminal backgroundColorCode],
                                &len,
                                ambiguousIsDoubleWidth,
                                NULL,
                                NULL,
                                NO,
                                unicodeVersion,
                                alternateScreen,
                                &rtlFound);
            if (rtlFound && rtlFoundPtr != nil) {
                *rtlFoundPtr = YES;
            }
            if ([token isAscii] && [terminal charset]) {
                ConvertCharsToGraphicsCharset(screenChars, len);
            }
            [result appendBytes:screenChars
                         length:sizeof(screen_char_t) * len];
            free(screenChars);
        }
        [token release];
    }
    CVectorDestroy(&vector);

    return result;
}

// Return an NSArray of NSData's. Each NSData is an array of screen_char_t's,
// with the last element in each being the newline. Returns nil on error.
- (NSArray<NSData *> *)parseDumpHistoryResponse:(NSString *)response
                         ambiguousIsDoubleWidth:(BOOL)ambiguousIsDoubleWidth
                                 unicodeVersion:(NSInteger)unicodeVersion
                                alternateScreen:(BOOL)alternateScreen
                                       rtlFound:(BOOL *)rtlFoundPtr {
    if (rtlFoundPtr) {
        *rtlFoundPtr = NO;
    }
    if (![response length]) {
        return [NSArray array];
    }
    NSArray *lines = [response componentsSeparatedByString:@"\n"];
    NSMutableArray *screenLines = [NSMutableArray array];
    VT100Terminal *terminal = [[[VT100Terminal alloc] init] autorelease];
    terminal.tmuxMode = YES;
    [terminal setEncoding:NSUTF8StringEncoding];
    for (NSString *line in lines) {
        BOOL rtlFound = NO;
        NSData *data = [self dataForHistoryLine:line
                                   withTerminal:terminal
                         ambiguousIsDoubleWidth:ambiguousIsDoubleWidth
                                 unicodeVersion:unicodeVersion
                                alternateScreen:alternateScreen
                                       rtlFound:&rtlFound];
        if (rtlFoundPtr != nil && rtlFound) {
            *rtlFoundPtr = YES;
        }
        if (!data) {
            return nil;
        }
        [screenLines addObject:data];
    }

    return screenLines;
}

@end
