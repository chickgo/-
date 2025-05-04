package com.klpbbs.service;

import com.klpbbs.model.File;
import com.klpbbs.model.User;
import com.klpbbs.repository.FileRepository;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;
import org.springframework.web.multipart.MultipartFile;

import java.io.File as IoFile;
import java.io.IOException;
import java.time.LocalDateTime;
import java.util.List;

@Service
public class FileService {

    @Autowired
    private FileRepository fileRepository;

    public File uploadFile(MultipartFile multipartFile, Long userId) {
        try {
            // 保存文件到服务器
            String filename = multipartFile.getOriginalFilename();
            IoFile file = new IoFile("uploads/" + filename);
            multipartFile.transferTo(file);

            // 保存文件记录到数据库
            File dbFile = new File();
            dbFile.setFilename(filename);
            dbFile.setPath(file.getAbsolutePath());
            dbFile.setUploadDate(LocalDateTime.now());

            User user = new User();
            user.setId(userId);
            dbFile.setUploader(user);

            return fileRepository.save(dbFile);
        } catch (IOException e) {
            e.printStackTrace();
            return null;
        }
    }

    public List<File> getFilesByUser(Long userId) {
        return fileRepository.findByUploaderId(userId);
    }

    public File getFileById(Long fileId) {
        return fileRepository.findById(fileId).orElse(null);
    }
}