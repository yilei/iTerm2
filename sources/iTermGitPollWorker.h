//
//  iTermGitPollWorker.h
//  iTerm2SharedARC
//
//  Created by George Nachman on 9/7/18.
//

#import <Foundation/Foundation.h>
#import "iTermGitState.h"

NS_ASSUME_NONNULL_BEGIN

@interface iTermGitPollWorker : NSObject
+ (instancetype)sharedInstance;
- (void)requestPath:(NSString *)path completion:(void (^)(iTermGitState * _Nullable))completion;
- (void)invalidateCacheForPath:(NSString *)path;
- (NSString *)cachedBranchForPath:(NSString *)path;
- (NSString *)debugInfoForDirectory:(NSString *)path;

@end

NS_ASSUME_NONNULL_END
