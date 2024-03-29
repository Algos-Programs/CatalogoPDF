//
//  PSPDFAnnotation.h
//  PSPDFKit
//
//  Copyright 2011-2012 Peter Steinberger. All rights reserved.
// 

#import "PSPDFKitGlobal.h"
#import "PSPDFConverter.h"
#import "UIColor+PSPDFKitAdditions.h"

@class PSPDFDocument;

// list of editable annotation types.
extern NSString *const PSPDFAnnotationTypeStringHighlight;
extern NSString *const PSPDFAnnotationTypeStringUnderline;
extern NSString *const PSPDFAnnotationTypeStringStrikeout;
extern NSString *const PSPDFAnnotationTypeStringNote;
extern NSString *const PSPDFAnnotationTypeStringFreeText;
extern NSString *const PSPDFAnnotationTypeStringInk;

// Annotations defined after the PDF standard.
typedef NS_OPTIONS(NSUInteger, PSPDFAnnotationType) {
    PSPDFAnnotationTypeUndefined = 0,      // any annotation whose type couldn't be recognized.
    PSPDFAnnotationTypeLink      = 1 << 1,
    PSPDFAnnotationTypeHighlight = 1 << 2, // (Highlight, Underline, StrikeOut) - PSPDFHighlightAnnotationView
    PSPDFAnnotationTypeText      = 1 << 5,
    PSPDFAnnotationTypeInk       = 1 << 6,
    PSPDFAnnotationTypeShape     = 1 << 7, // Square, Circle
    PSPDFAnnotationTypeLine      = 1 << 8,
    PSPDFAnnotationTypeNote      = 1 << 9,
    PSPDFAnnotationTypeAll       = ULONG_MAX
};

/**
    Defines a PDF annotation.
    PSPDFAnnotationParser searches the runtime for subclasses of PSPDFAnnotation and builds up a dictionary using supportedTypes.
 
    Don't directly make an instance of this class, use the subclasses like PSPDFNoteAnnotations, PSPDFLinkAnnotations or others. This class will return nil if initialized directly, unless with the type PSPDFAnnotationTypeUndefined.

    Subclasses need to implement - (id)initWithAnnotationDictionary:(CGPDFDictionaryRef)annotationDictionary inAnnotsArray:(CGPDFArrayRef)annotsArray
*/
@interface PSPDFAnnotation : NSObject <NSCoding, NSCopying> {
    UIColor *_color;
    CGRect _boundingBox;
    int _popupIndex;
    int _indexOnPage;
    float _alpha;
}

/// Returns the annotation type strings that are supported. Implemented in each subclass.
+ (NSArray *)supportedTypes;

/// Returns YES if PSPDFKit has support to write this annotation back into the PDF.
+ (BOOL)isWriteable;

/// Use this to create custom user annotations. 
- (id)initWithType:(PSPDFAnnotationType)annotationType;

/// Used for generic PSPDFAnnotations (those that are not recognized by PSPDFAnnotationParser)
/// Implement this in your subclass.
- (id)initWithAnnotationDictionary:(CGPDFDictionaryRef)annotDict inAnnotsArray:(CGPDFArrayRef)annotsArray;

/// Initialize annotation with the corresponding PDF dictionary. Call from subclass.
- (id)initWithAnnotationDictionary:(CGPDFDictionaryRef)annotationDictionary inAnnotsArray:(CGPDFArrayRef)annotsArray type:(PSPDFAnnotationType)annotationType;

/// To edit annotations, you need to make a copy and delete the original.
/// Returns same type as current class.
- (id)copyAndDeleteOriginalIfNeeded;

/// Check if point is inside annotation area.
- (BOOL)hitTest:(CGPoint)point;

/// Calculates the exact annotation position in the current page.
- (CGRect)rectForPageRect:(CGRect)pageRect;

- (NSComparisonResult)compareByPositionOnPage:(PSPDFAnnotation *)otherAnnotation;
- (CGRect)rectFromPDFArray:(CGPDFArrayRef)array;
- (NSArray *)rectsFromQuadPointsInArray:(CGPDFArrayRef)quadPointsArray;

/**
    Draw current annotation in context.
    Coordinates here are in PDF coordinate space.

    Use PSPDFConvertViewRectToPDFRect to convert your coordinates accordingly.
    (For performance considerations, you want to do this once, not every time drawInContext is called)
 */
- (void)drawInContext:(CGContextRef)context;

/// Returns NSData string represnetation in the PDF Standard.
- (NSData *)pdfDataRepresentation;

/// Current annotation type. 
@property(nonatomic, assign, readonly) PSPDFAnnotationType type;

/// If YES, the annotation will be rendered as a overlay. If NO, it will be statically rendered within the PDF content image.
/// PSPDFAnnotationTypeLink and PSPDFAnnotationTypeNote currently are rendered as overlay.
/// Currently won't work if you just set arbitrary annotations to overlay=YES.
/// If overlay is set to yes, you must also register the corresponding *AnnotationView class to render (override PSPDFAnnotationParser's annotationClassForAnnotation)
@property(nonatomic, assign, getter=isOverlay, readonly) BOOL overlay;

/// Annotation type string as defined in the PDF.
/// Usually read from the annotDict. Don't change this unless you know what you're doing.
@property(nonatomic, copy) NSString *typeString;

/// Alpha value of the annotation color.
@property (nonatomic, assign) float alpha;

/// Color associated with the annotation or nil if there is no color.
/// Note: use .alpha for transparency, not the alpha value in color.
@property(nonatomic, strong) UIColor *color;

/// Color with added alpha value.
@property(nonatomic, strong, readonly) UIColor *colorWithAlpha;

/// Optional. Various annotation types may contain text.
@property(nonatomic, strong) NSString *contents;

/// Border Line Width (only used in certain annotations)
@property (nonatomic, assign) float lineWidth;

@property(nonatomic, assign) int indexOnPage;

/// Some annotations may have a popupIndex. Defaults to -1.
@property(nonatomic, assign) int popupIndex;

/// Annotation may already be deleted locally, but not written back.
@property(nonatomic, assign, getter=isDeleted) BOOL deleted;

/// rectangle of specific annotation.
@property(nonatomic, assign) CGRect boundingBox;

/// page for current annotation.
@property(nonatomic, assign) NSUInteger page;

/// If this annotation isn't backed by the PDF, it's dirty by default.
/// After the annotation has been written to the file, this will be reset until the annotation has been changed.
@property(nonatomic, assign, getter=isDirty) BOOL dirty;

/// corresponding document, weak.
@property(nonatomic, ps_weak) PSPDFDocument *document;

@end


@interface PSPDFAnnotation (PSPDFAnnotationWriting)

// PDF rect string representation (/Rect [%f %f %f %f])
- (NSString *)pdfRectString;

// Color string representation (/C [%f %f %f])
- (NSString *)pdfColorString;

// Color string representation (/C [%f %f %f] /CA %f)
- (NSString *)pdfColorWithAlphaString;

// Appends escaped contents data if contents length is > 0.
- (void)appendEscapedContents:(NSMutableData *)pdfData;

// Converts an array of NSValue-CGRect's into an array of CGRect-NSString's.
+ (NSArray *)stringsFromRectsArray:(NSArray *)rects;

// Converts an array of CGRect-NSString's into a array of NSValue-CGRect's.
+ (NSArray *)rectsFromStringsArray:(NSArray *)rectStrings;

@end
