#ifndef VPREVIEWMANAGER_H
#define VPREVIEWMANAGER_H

#include <QObject>
#include <QString>
#include <QTextBlock>
#include <QHash>
#include <QVector>
#include <QSharedPointer>

#include "hgmarkdownhighlighter.h"
#include "vmdeditor.h"
#include "vtextblockdata.h"

class VDownloader;

typedef long long TS;

// Info about image to preview.
struct VImageToPreview
{
    void clear()
    {
        m_startPos = m_endPos = m_blockPos = m_blockNumber = -1;
        m_padding = 0;
        m_image = QPixmap();
        m_name.clear();
        m_isBlock = true;
    };

    int m_startPos;

    int m_endPos;

    // Position of this block.
    int m_blockPos;

    int m_blockNumber;

    // Left padding of this block in pixels.
    int m_padding;

    QPixmap m_image;

    // If @m_name are the same, then they are the same imges.
    QString m_name;

    // Whether it is an image block.
    bool m_isBlock;
};


// Manage inplace preview.
class VPreviewManager : public QObject
{
    Q_OBJECT
public:
    VPreviewManager(VMdEditor *p_editor, HGMarkdownHighlighter *p_highlighter);

    void setPreviewEnabled(bool p_enabled);

    // Clear all the preview.
    void clearPreview();

    // Refresh all the preview.
    void refreshPreview();

    bool isPreviewEnabled() const;

    // Calculate the block margin (prefix spaces) in pixels.
    static int calculateBlockMargin(const QTextBlock &p_block, int p_tabStopWidth);

public slots:
    // Image links were updated from the highlighter.
    void updateImageLinks(const QVector<VElementRegion> &p_imageRegions);

    void updateCodeBlocks(const QVector<QSharedPointer<VImageToPreview> > &p_images);

signals:
    // Request highlighter to update image links.
    void requestUpdateImageLinks();

    void previewEnabledChanged(bool p_enabled);

private slots:
    // Non-local image downloaded for preview.
    void imageDownloaded(const QByteArray &p_data, const QString &p_url);

private:
    struct ImageLinkInfo
    {
        ImageLinkInfo()
            : m_startPos(-1),
              m_endPos(-1),
              m_blockPos(-1),
              m_blockNumber(-1),
              m_padding(0),
              m_isBlock(false)
        {
        }

        ImageLinkInfo(int p_startPos,
                      int p_endPos,
                      int p_blockPos,
                      int p_blockNumber,
                      int p_padding)
            : m_startPos(p_startPos),
              m_endPos(p_endPos),
              m_blockPos(p_blockPos),
              m_blockNumber(p_blockNumber),
              m_padding(p_padding),
              m_isBlock(false)
        {
        }

        int m_startPos;

        int m_endPos;

        // Position of this block.
        int m_blockPos;

        int m_blockNumber;

        // Left padding of this block in pixels.
        int m_padding;

        // Short URL within the () of ![]().
        // Used as the ID of the image.
        QString m_linkShortUrl;

        // Full URL of the link.
        QString m_linkUrl;

        // Whether it is an image block.
        bool m_isBlock;
    };

    // Start to preview images according to image links.
    void previewImages(TS p_timeStamp, const QVector<VElementRegion> &p_imageRegions);

    // According to p_imageRegions, fetch the image link Url.
    // @p_imageRegions: output.
    void fetchImageLinksFromRegions(QVector<VElementRegion> p_imageRegions,
                                    QVector<ImageLinkInfo> &p_imageLinks);

    // Fetch the image link's URL if there is only one link.
    QString fetchImageUrlToPreview(const QString &p_text);

    // Fetch teh image's full path if there is only one image link.
    // @p_url: contains the short URL in ![]().
    QString fetchImagePathToPreview(const QString &p_text, QString &p_url);

    // Update the preview info of related blocks according to @p_imageLinks.
    void updateBlockPreviewInfo(TS p_timeStamp, const QVector<ImageLinkInfo> &p_imageLinks);

    // Update the preview info of related blocks according to @p_images.
    void updateBlockPreviewInfo(TS p_timeStamp,
                                PreviewSource p_source,
                                const QVector<QSharedPointer<VImageToPreview> > &p_images);

    // Get the name of the image in the resource manager.
    // Will add the image to the resource manager if not exists.
    // Returns empty if fail to add the image to the resource manager.
    QString imageResourceName(const ImageLinkInfo &p_link);

    QString imageResourceNameFromCodeBlock(const QSharedPointer<VImageToPreview> &p_image);

    QHash<QString, long long> &imageCache(PreviewSource p_source);

    void clearObsoleteImages(long long p_timeStamp, PreviewSource p_source);

    void clearBlockObsoletePreviewInfo(long long p_timeStamp, PreviewSource p_source);

    TS &timeStamp(PreviewSource p_source);

    VMdEditor *m_editor;

    QTextDocument *m_document;

    HGMarkdownHighlighter *m_highlighter;

    VDownloader *m_downloader;

    // Whether preview is enabled.
    bool m_previewEnabled;

    // Map from URL to name in the resource manager.
    // Used for downloading images.
    QHash<QString, QString> m_urlToName;

    // Timestamp per each preview source.
    TS m_timeStamps[(int)PreviewSource::MaxNumberOfSources];

    // Used to discard obsolete images. One per each preview source.
    QHash<QString, long long> m_imageCaches[(int)PreviewSource::MaxNumberOfSources];
};

inline QHash<QString, long long> &VPreviewManager::imageCache(PreviewSource p_source)
{
    return m_imageCaches[(int)p_source];
}

inline TS &VPreviewManager::timeStamp(PreviewSource p_source)
{
    return m_timeStamps[(int)p_source];
}

inline bool VPreviewManager::isPreviewEnabled() const
{
    return m_previewEnabled;
}
#endif // VPREVIEWMANAGER_H
